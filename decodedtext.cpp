#include "decodedtext.h"

#include <QStringList>
#include <QRegularExpression>
#include <QRegExp>
#include <QDebug>
#include <QDateTime>

extern "C" {
  bool stdmsg_(char const * msg, fortran_charlen_t);
}

namespace
{
  QRegularExpression words_re {R"(^(?:(?<word1>(?:CQ|DE|QRZ)(?:\s?DX|\s(?:[A-Z]{2}|\d{3}))|[A-Z0-9/]+)\s)(?:(?<word2>[A-Z0-9/]+)(?:\s(?<word3>[-+A-Z0-9]+)(?:\s(?<word4>(?:OOO|(?!RR73)[A-R]{2}[0-9]{2})))?)?)?)"};
  QRegularExpression grid_regexp {"\\A(?![Rr]{2}73)[A-Ra-r]{2}[0-9]{2}([A-Xa-x]{2}){0,1}\\z"};
  QRegularExpression field_day_exchange_re {
    R"(
        (
           [1-9]                          # # transmitters (1 to 32 inc.)
          |[0-2]\d
          |3[0-2]
        )
        [A-F]\ *                          # class and optional space
        (
           AB|AK|AL|AR|AZ|BC|CO|CT|DE|EB  # ARRL/RAC section
          |EMA|ENY|EPA|EWA|GA|GTA|IA|ID
          |IL|IN|KS|KY|LA|LAX|MAR|MB|MDC
          |ME|MI|MN|MO|MS|MT|NC|ND|NE|NFL
          |NH|NL|NLI|NM|NNJ|NNY|NT|NTX|NV
          |OH|OK|ONE|ONN|ONS|OR|ORG|PAC
          |PR|QC|RI|SB|SC|SCV|SD|SDG|SF
          |SFL|SJV|SK|SNJ|STX|SV|TN|UT|VA
          |VI|VT|WCF|WI|WMA|WNY|WPA|WTX
          |WV|WWA|WY
          |DX                             # anyone else
        )
      )", QRegularExpression::CaseInsensitiveOption | QRegularExpression::ExtendedPatternSyntaxOption};
}

DecodedText::DecodedText (QString const& the_string, Configuration::SpecialOperatingActivity specOp)
    //create dummy decode if decode text smaller than that of the shortest real decode text
  : string_ {(the_string.size() < 28 and the_string.size()) ? "1200 -01 -0.1 1500 #  " + the_string : the_string.left (the_string.indexOf (QChar::Nbsp))} // discard appended info
  , padding_ {string_.indexOf (" ") > 4 ? 2 : 0} // allow for
                                                    // seconds
  , message_ {string_.mid (column_qsoText + padding_).trimmed ()}
  , is_standard_ {false}
  , valid_ {false}
{
//  qDebug () << "DecodedText: the_string:" << the_string << "Nbsp pos:" << the_string.indexOf (QChar::Nbsp);
  if (message_.length() >= 1)
    {
       message0_ = message_.left(36);
       message_ = message_.left(36).remove (QRegularExpression {"[<>]"});
      int i1 = message_.indexOf ('\r');
      if (i1 > 0)
        {
          message_ = message_.left (i1 - 1);
        }
      if (message_.contains (QRegularExpression {"^(CQ|QRZ)\\s"}))
        {
          // TODO this magic position 16 is guaranteed to be after the
          // last space in a decoded CQ or QRZ message but before any
          // appended DXCC entity name or worked before information
          auto eom_pos = message_.indexOf (' ', 16);
          // we always want at least the characters to position 16
          if (eom_pos < 16) eom_pos = message_.size () - 1;
          // remove DXCC entity and worked B4 status. TODO need a better way to do this
          message_ = message_.left (eom_pos + 1);
        }
      // stdmsg is a Fortran routine that packs the text, unpacks it
      // and compares the result
      auto message_c_string = message0_.toLocal8Bit ();
      message_c_string += QByteArray {37 - message_c_string.size (), ' '};
      is_standard_ = stdmsg_(message_c_string.constData(),37);
      
      toCall_ = "";
      deCall_ = "";
      report_ = "";
      rogerReport_ = "";
      exch_ = "";
      rogerExch_ = "";
      grid_ = "";
      dirTo_ = "";
      valid_ = false;
      hash_ = message_.contains("...");
      
      //avt 4/8/25
      if (!hash_) {
        words_ = message_.split (' ', QString::SkipEmptyParts);
        if (words_.size() >= 2) {
          toCall_ = words_[0];
          deCall_ = words_[1];
          
          if (specOp == Configuration::SpecialOperatingActivity::NONE or specOp == Configuration::SpecialOperatingActivity::POTA) {
            //WM8Q K1JT
            //CQ   WM8Q
            //WM8Q K1JT RRR
            //WM8Q K1JT RR73
            //WM8Q K1JT EM45 
            //WM8W K1JT +10 
            //CQ   WM8Q DN61
            //CQ   DX   WM8Q
            //CQ   DX   WM8Q DN61
            // 0    1     2   3
            if (words_.size() >= 3) {
              if (!words_[1].contains(QRegExp("[0-9]"))) {   //2nd word alpha only
                deCall_ = words_[2];
                if (words_[1].size() >= 2 and words_[1].size() <= 4) dirTo_ = words_[1];
                if (words_.size() == 4) msgData_ = words_[3];
              } else {  //2nd word has a number
                msgData_ = words_[2];
              }
            }
            
            if (msgData_.contains(QRegExp("[0-9]")) and (msgData_.contains("+") or msgData_.contains("-")) and (!msgData_.contains("R"))) report_ = msgData_;
            if (msgData_.contains(QRegExp("[0-9]")) and (msgData_.contains("R+") or msgData_.contains("R-"))) rogerReport_ = msgData_;
            if (msgData_.contains(grid_regexp)) grid_ = msgData_;
        
            valid_ = (toCall_ == "CQ" or toCall_.contains(QRegExp("[0-9]")))
              and deCall_.contains(QRegExp("[0-9]"))
              and dirTo_ != "FD"
              and ((toCall_ == "CQ" and words_.size() == 2)
                  or !report_.isEmpty()
                  or !rogerReport_.isEmpty()
                  or !grid_.isEmpty()
                  or msgData_ == "RRR"
                  or msgData_ == "73"
                  or msgData_ == "RR73");
              
          } else if (specOp == Configuration::SpecialOperatingActivity::FIELD_DAY) {
            //CQ   FD   WM8Q DN61
            //WM8Q K1JT RRR
            //WM8Q K1JT RR73
            //WM8Q K1JT 2B   WY
            //K1JT WM8Q R    2B  WY
            // 0     1  2     3   4
    
            if (words_.size() == 3 and (words_[2] == "73" or words_[2] == "RR73" or words_[2] == "RRR")) msgData_ = words_[2];
            if (words_.size() == 4 and words_[0] == "CQ" and words_[1] == "FD") {
              msgData_ = words_[3];
              if (msgData_.contains(grid_regexp)) grid_ = msgData_;
              deCall_ = words_[2];
              dirTo_ = words_[1];
            }
            if (words_.size() >= 4) {
              if (words_[2] == "R" and words_.size() == 5) {
                msgData_ = words_[3] + " " +  words_[4];
                if (msgData_.contains(field_day_exchange_re)) {
                  rogerExch_ = msgData_;
                  rogerReport_ = words_[3];   //not actual SNR, but it gets logged this way
                }
              } else {
                msgData_ = words_[2] + " " +  words_[3];
                if (msgData_.contains(field_day_exchange_re)) {
                  exch_ = msgData_;
                  report_ = words_[2];        //not actual SNR, but it gets logged this way
                }
              }
            }
            
            valid_ = (toCall_ == "CQ" or toCall_.contains(QRegExp("[0-9]")))
              and deCall_.contains(QRegExp("[0-9]"))
              and (!exch_.isEmpty()
                  or !rogerExch_.isEmpty()
                  or !grid_.isEmpty()
                  or msgData_ == "RRR"
                  or msgData_ == "73"
                  or msgData_ == "RR73");                
          }
        }
      }
    }
};

QStringList DecodedText::messageWords () const
{
  if (is_standard_)
    {
      // extract up to the first four message words
      return words_re.match (message_).capturedTexts ();
    }
  // simple word split for free text messages
  auto words = message_.split (' ', QString::SkipEmptyParts);
  // add whole message as item 0 to mimic RE capture list
  words.prepend (message_);
  return words;
}

QString DecodedText::CQersCall() const
{
  QRegularExpression callsign_re {R"(^(CQ|DE|QRZ)(\s?DX|\s([A-Z]{2,4}|\d{3}))?\s(?<callsign>[A-Z0-9/]{2,})(\s[A-R]{2}[0-9]{2})?)"};
  return callsign_re.match (message_).captured ("callsign");
}


bool DecodedText::isJT65() const
{
    return string_.indexOf("#") == column_mode + padding_;
}

bool DecodedText::isJT9() const
{
    return string_.indexOf("@") == column_mode + padding_;
}

bool DecodedText::isTX() const
{
    int i = string_.indexOf("Tx");
    return (i >= 0 && i < 15); // TODO guessing those numbers. Does Tx ever move?
}

bool DecodedText::isLowConfidence () const
{
  return QChar {'?'} == string_.mid (padding_ + column_qsoText + 21, 1);
}

int DecodedText::frequencyOffset() const
{
    return string_.mid(column_freq + padding_,4).toInt();
}

int DecodedText::snr() const
{
  int i1=string_.indexOf(" ")+1;
  return string_.mid(i1,3).toInt();
}

float DecodedText::dt() const
{
  return string_.mid(column_dt + padding_,5).toFloat();
}

/*
2343 -11  0.8 1259 # YV6BFE F6GUU R-08
2343 -19  0.3  718 # VE6WQ SQ2NIJ -14
2343  -7  0.3  815 # KK4DSD W7VP -16
2343 -13  0.1 3627 @ CT1FBK IK5YZT R+02

0605  Tx      1259 # CQ VK3ACF QF22
*/

// find and extract any report. Returns true if this is a standard message
bool DecodedText::report(QString const& myBaseCall, QString const& dxBaseCall, /*mod*/QString& report) const
{
  if (message_.size () < 1) return false;

  QStringList const& w = message_.split(" ",QString::SkipEmptyParts);
  if (w.size ()
      && is_standard_ && (w[0] == myBaseCall
                          || w[0].endsWith ("/" + myBaseCall)
                          || w[0].startsWith (myBaseCall + "/")
                          || (w.size () > 1 && !dxBaseCall.isEmpty ()
                              && (w[1] == dxBaseCall
                                  || w[1].endsWith ("/" + dxBaseCall)
                                  || w[1].startsWith (dxBaseCall + "/")))))
    {
      QString tt="";
      if(w.size() > 2) tt=w[2];
      bool ok;
      auto i1=tt.toInt(&ok);
      if (ok and i1>=-50 and i1<50)
        {
          report = tt;
        }
      else
        {
          if (tt.mid(0,1)=="R")
            {
              i1=tt.mid(1).toInt(&ok);
              if(ok and i1>=-50 and i1<50)
                {
                  report = tt.mid(1);
                }
            }
        }
    }
  return is_standard_;
}

unsigned DecodedText::timeInSeconds() const
{
  return 3600 * string_.mid (column_time, 2).toUInt ()
    + 60 * string_.mid (column_time + 2, 2).toUInt()
    + (padding_ ? string_.mid (column_time + 2 + padding_, 2).toUInt () : 0U);
}

//age in seconds, up to 1 day max
int DecodedText::ageSec() const
{
  auto now = QDateTime::currentDateTimeUtc();
  int nowSec = now.time().hour() * 3600 + now.time().minute() * 60 + now.time().second();   //since last midnight. 0 - 86400
  int t = timeInSeconds();      //since last midnight. 0 - 86400
  if (t > nowSec) t -= 86400;
  return nowSec - t;
}

/*
2343 -11  0.8 1259 #  YV6BFE F6GUU R-08
2343 -19  0.3  718 #  VE6WQ SQ2NIJ -14
2343  -7  0.3  815 #  KK4DSD W7VP -16
2343 -13  0.1 3627 @  CT1FBK IK5YZT R+02
0605  Tx      1259 #  CQ VK3ACF QF22
*/

QString DecodedText::report() const // returns a string of the SNR field with a leading + or - followed by two digits
{
    int sr = snr();
    if (sr<-50)
        sr = -50;
    else
        if (sr > 49)
            sr = 49;

    QString rpt;
    rpt.sprintf("%d",abs(sr));
    if (sr > 9)
        rpt = "+" + rpt;
    else
        if (sr >= 0)
            rpt = "+0" + rpt;
        else
            if (sr >= -9)
                rpt = "-0" + rpt;
            else
                rpt = "-" + rpt;
    return rpt;
}

bool DecodedText::is_cq() const {
  return valid_ and toCall_ == "CQ" and !deCall_.isEmpty();
}

bool DecodedText::is_reply() const {
  return valid_ and toCall_ != "CQ" and !toCall_.isEmpty() and !deCall_.isEmpty() and (!grid_.isEmpty() or (words_.size() == 2));
}

bool DecodedText::is_report() const {
  return valid_ and !toCall_.isEmpty() and !deCall_.isEmpty() and !report_.isEmpty(); 
}

bool DecodedText::is_rogerReport() const {
  return valid_ and !toCall_.isEmpty() and !deCall_.isEmpty() and !rogerReport_.isEmpty();
}

bool DecodedText::is_exch() const {
  return valid_ and !toCall_.isEmpty() and !deCall_.isEmpty() and !exch_.isEmpty(); 
}

bool DecodedText::is_rogerExch() const {
  return valid_ and !toCall_.isEmpty() and !deCall_.isEmpty() and !rogerExch_.isEmpty();
}

bool DecodedText::is_rogers() const {
  return valid_ and !toCall_.isEmpty() and !deCall_.isEmpty() and msgData_ == "RRR";
}

bool DecodedText::is_73() const {
  return valid_ and !toCall_.isEmpty() and !deCall_.isEmpty() and msgData_ == "73";
}

bool DecodedText::is_RR73() const {
  return valid_ and !toCall_.isEmpty() and !deCall_.isEmpty() and msgData_ == "RR73";
}

bool DecodedText::is_73orRR73() const {
  return valid_ and !toCall_.isEmpty() and !deCall_.isEmpty() and (is_73() or is_RR73());
}

bool DecodedText::is_hash() const {
  return hash_;
}

bool DecodedText::is_valid() const {
  return valid_;
}

bool DecodedText::is_std() const {
  return is_standard_;
}

QString DecodedText::reportSnr() const {
  return report_;
}

QString DecodedText::rogerReportSnr() const
{
  return rogerReport_;
}

QString DecodedText::anyReportSnr() const {
  return !report_.isEmpty() ? report_ : rogerReport_;
}

QString DecodedText::grid() const
{
  return grid_;
}

QString DecodedText::exch() const
{
  return exch_;
}

QString DecodedText::rogerExch() const
{
  return rogerExch_;
}

QString DecodedText::anyExch() const {
  return !exch_.isEmpty() ? exch_: rogerExch_;
}

QString DecodedText::dirTo() const
{
  return dirTo_;
}

QString DecodedText::toCall() const
{
    return toCall_;
}

QString DecodedText::deCall() const
{
  return deCall_;
}

QString DecodedText::message() const
{
  return message_;
}

QString DecodedText::debugInfo() const
{
  return "'" + words_.join(",") + "'  '" + msgData_ + "'";
}
