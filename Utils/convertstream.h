#ifndef CONVERTSTREAM_H
#define CONVERTSTREAM_H
//////////////////////////////////////////////////////
//
// $Id$
//
//////////////////////////////////////////////////////
//
// basic_iconvertstream, basic_oconvertstream
//
// Normal basic_stringstream do not apply the codecvt facet
// of their locale.  These two streams act exactly like
// basic_stringstream except that they do apply the imbued codecvt
// facet to their input (in the case of basic_iconvertstream) 
// or output (int the case of basic_oconvertstream).  
//
// This means you can to cool things like this
//
//  std::locale loc(std::_Addfac(std::locale(), new base64_codecvt));
//  converting_ostringstream os;
//  os.imbue(loc);
//
//  os << "stuff"; 
//     ...  lots more stuff streamed into os
//
//  std::cout << os.str() << std::endl;
//     os.str() contains the Base64 encoded byte sequence
//
// Decoding is just as simple.
//
//  std::locale loc(std::_Addfac(std::locale(), new base64_codecvt));
// 
//  converting_istringstream is;
//  is.imbue(loc);
//  is.str(a_base64_byte_stream);
//  std::cout << is.str();
//    ... is.str() is the decode byte stream, which can also be extracted 
//    ... using >> operators (is >> byte; etc)
//
////////////////////////////////////////////////////////////

#include <locale>
#include <sstream>
#include <algorithm>

template<typename charT, 
         typename traitsT = std::char_traits<charT>,
         typename fromCharT = charT,
         typename fromTraitsT = std::char_traits<fromCharT> >
class basic_iconvertstream : public std::basic_istream<charT, traitsT>
{
public:
  typedef std::basic_istream<charT, traitsT> istreamT;
  typedef std::basic_stringbuf<charT, traitsT> stringbufT;
  typedef std::basic_string<charT, traitsT> stringT;
  typedef std::basic_string<fromCharT, fromTraitsT> fromStringT;

  explicit basic_iconvertstream(ios_base::openmode mode = in)
		: std::basic_istream<charT, traitsT>(0), stringbuf_(mode | in)
    {
      istreamT::rdbuf(&stringbuf_);
    }
  explicit basic_iconvertstream(const stringT& str, ios_base::openmode mode = in)
		: std::basic_istream<charT, traitsT>(0), stringbuf_(mode | in) 
    {
      istreamT::rdbuf(&stringbuf_);
      str(str);
    }
  virtual ~basic_iconvertstream()
  {}

  stringbufT* rdbuf() const
  {
    return static_cast<stringbufT *>(&stringbuf_); 
  } // rdbuf

  stringT str() const
  {
    return stringbuf_.str(); 
  } // str

  void str(const fromStringT& str)
  {
    // do conversion
#ifndef _MSC_VER
    const std::codecvt<charT, fromCharT, typename traitsT::state_type>& cvt =
      std::use_facet<std::codecvt<charT, fromCharT, typename traitsT::state_type> >(this->getloc());
#else
    const std::codecvt<charT, fromCharT, traitsT::state_type>& cvt =
      std::use_facet(stringbuf_.getloc(), (std::codecvt<charT, fromCharT, traitsT::state_type>*)0, true);
#endif

    if(cvt.always_noconv())
    {
      stringbuf_.str(no_conversion(str)); 
      return;
    }

    // we must do code conversion
    stringT converted;
    int toBufLen = str.length() + 4; // 4 is arbitrary bit of bonus space
    charT* to = new charT[toBufLen]; // 4 is arbitrary
    const fromCharT* from_next = str.data();
    typename std::codecvt_base::result r;
    typename traitsT::state_type state;

    do
    {
      charT* to_next;
      r = cvt.in(state, from_next, str.data() + str.length(), from_next, 
                  to, to + toBufLen, to_next);
      if(r == std::codecvt_base::noconv)
      {
        converted.append(no_conversion(str));
        break;
      }
      converted.append(to, (to_next - to));
    }
    while(r == std::codecvt_base::partial);

    delete[] to;
    // naughty! ignore (r == std::codecvt_base::error)

    stringbuf_.str(converted); 
  } // str

private:
  stringT no_conversion(const fromStringT& str)
  {
    stringT dest;
    std::copy(str.begin(), str.end(), std::back_inserter(dest));
    return dest;
  } // no_conversion

  stringbufT stringbuf_;
}; // basic_iconvertstream

template<typename charT, 
         typename traitsT = std::char_traits<charT>,
         typename toCharT = charT,
         typename toTraitsT = std::char_traits<toCharT> >
class basic_oconvertstream : public std::basic_ostream<charT, traitsT> 
{
public:
  typedef std::basic_ostream<charT, traitsT> ostreamT;
  typedef std::basic_stringbuf<charT, traitsT> stringbufT;
  typedef std::basic_string<charT, traitsT> stringT;
  typedef std::basic_string<toCharT, toTraitsT> toStringT;

  explicit basic_oconvertstream(ios_base::openmode mode = out)
		: std::basic_ostream<charT, traitsT>(0), stringbuf_(mode | out)
    {
      ostreamT::rdbuf(&stringbuf_);
    }
  explicit basic_oconvertstream(const stringT& str, ios_base::openmode mode = out)
		: std::basic_ostream<charT, traitsT>(0), stringbuf_(str, mode | out) 
    {
      ostreamT::rdbuf(&stringbuf_);
    }
  virtual ~basic_oconvertstream()
    {}

  stringbufT* rdbuf() const
  {
    return static_cast<stringbufT *>(&stringbuf_); 
  } // rdbuf

  toStringT str()
  {
    toStringT out;
    stringT newstuff(stringbuf_.str()); 

    if(newstuff.length() == 0)
      return out;

    // convert it here
#ifndef _MSC_VER
    const std::codecvt<charT, toCharT, typename traitsT::state_type>& cvt =
      std::use_facet<std::codecvt<charT, toCharT, typename traitsT::state_type> >(this->getloc());
#else
    const std::codecvt<charT, toCharT, traitsT::state_type>& cvt =
      std::use_facet(stringbuf_.getloc(), (std::codecvt<charT, toCharT, traitsT::state_type>*)0, true);
#endif

    if(cvt.always_noconv())
      out.append(no_conversion(newstuff));
    else
    {
      // we must do code conversion
      int toBufLen = newstuff.length() + 4; // 4 is arbitrary little bit of extra space
      toCharT* to = new toCharT[toBufLen];  
      const charT* from_next = newstuff.data();
      typename std::codecvt_base::result r;
      typename traitsT::state_type state;

      do
      {
        toCharT* to_next;
        r = cvt.out(state, from_next, newstuff.data() + newstuff.length(), from_next, 
                    to, to + toBufLen, to_next);
        if(r == std::codecvt_base::noconv)
        {
          out.append(no_conversion(newstuff));
          break;
        }
        out.append(to, (to_next - to));
      }
      while(r == std::codecvt_base::partial);

      delete[] to;
      // naughty! ignore (r == std::codecvt_base::error)
    } // if(cvt.always_noconv())

    stringbuf_.str(stringT());

    return out;
  } // str

  void str(const stringT& str)
  {
    stringbuf_.str(str); 
  } // str

private:
  toStringT no_conversion(const stringT& str)
  {
    stringT dest;
    std::copy(str.begin(), str.end(), std::back_inserter(dest));
    return dest;
  } // no_conversion

  stringbufT stringbuf_;
}; // basic_oconvertstream

typedef basic_iconvertstream<char> converting_istringstream;
typedef basic_iconvertstream<wchar_t> converting_iwstringstream;
typedef basic_oconvertstream<char> converting_ostringstream;
typedef basic_oconvertstream<wchar_t> converting_owstringstream;

#endif