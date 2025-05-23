// -*- Mode: C++ -*-
/*
 * Class to handle the formatted string as returned from the fortran decoder
 *
 * VK3ACF August 2013
 */


#ifndef DECODEDTEXT_H
#define DECODEDTEXT_H

#include <QString>
#include <QStringList>

#include "Configuration.hpp"

/*
012345678901234567890123456789012345678901
^    ^    ^   ^    ^  ^
2343 -11  0.8 1259 #  CQ VP2X/GM4WJS GL33
2343 -11  0.8 1259 #  CQ 999 VP2V/GM4WJS
2343 -11  0.8 1259 #  YV6BFE F6GUU R-08
2343 -19  0.3  718 #  VE6WQ SQ2NIJ -14
2343  -7  0.3  815 #  KK4DSD W7VP -16
2343 -13  0.1 3627 @  CT1FBK IK5YZT R+02

0605  Tx      1259 #  CQ VK3ACF QF22
*/

class DecodedText
{
public:
  explicit DecodedText (QString const& message, Configuration::SpecialOperatingActivity specOp);

  QString string() const { return string_; };
  QStringList messageWords () const;
  int indexOf(QString s) const { return string_.indexOf(s); };
  int indexOf(QString s, int i) const { return string_.indexOf(s,i); };
  QString mid(int f, int t) const { return string_.mid(f,t); };
  QString left(int i) const { return string_.left(i); };

  void clear() { string_.clear(); };

  QString CQersCall() const;

  bool isJT65() const;
  bool isJT9() const;
  bool isTX() const;
  bool isStandardMessage () const {return is_standard_;}
  bool isLowConfidence () const;
  int frequencyOffset() const;  // hertz offset from the tuned dial or rx frequency, aka audio frequency
  int snr() const;
  float dt() const;

  // find and extract any report. Returns true if this is a standard message
  bool report(QString const& myBaseCall, QString const& dxBaseCall, /*mod*/QString& report) const;

  unsigned timeInSeconds() const;

  // returns a string of the SNR field with a leading + or - followed by two digits
  QString report() const;
  
  //avt 4/8/25
  bool is_cq() const;
  bool is_reply() const;
  bool is_report() const;
  bool is_rogerReport() const;
  bool is_exch() const;
  bool is_rogerExch() const;
  bool is_rogers() const;
  bool is_73() const;
  bool is_RR73() const;
  bool is_73orRR73() const;
  bool is_hash() const;
  bool is_valid() const;
  bool is_std() const;
  
  QString dirTo() const; 
  QString toCall() const; 
  QString deCall() const;
  QString message() const;
  
  QString reportSnr() const;
  QString rogerReportSnr() const;
  QString anyReportSnr() const;
  QString grid() const;
  QString exch() const;
  QString rogerExch() const;
  QString anyExch() const;
  
  int ageSec() const;
  
  QString debugInfo() const;

private:
  // These define the columns in the decoded text where fields are to be found.
  // We rely on these columns being the same in the fortran code (lib/decoder.f90) that formats the decoded text
  enum Columns {column_time    = 0,
      column_snr     = 5,
      column_dt      = 9,
      column_freq    = 14,
      column_mode    = 19,
      column_qsoText = 22 };

  QString string_;
  int padding_;
  bool contest_mode_;
  QString message_;
  QString message0_;
  bool is_standard_;
  
  //avt 4/8/25
  QStringList words_;
  QString toCall_;
  QString deCall_;
  QString report_;
  QString rogerReport_;
  QString exch_;
  QString rogerExch_;
  QString grid_;
  QString dirTo_;
  QString msgData_;
  bool valid_;
  bool hash_;
};

#endif // DECODEDTEXT_H
