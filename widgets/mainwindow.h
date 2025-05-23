// -*- Mode: C++ -*-
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QThread>
#include <QProcess>
#include <QProgressBar>
#include <QTimer>
#include <QDateTime>
#include <QList>
#include <QAudioDeviceInfo>
#include <QScopedPointer>
#include <QDir>
#include <QProgressDialog>
#include <QAbstractSocket>
#include <QHostAddress>
#include <QPointer>
#include <QSet>
#include <QVector>
#include <QFuture>
#include <QFutureWatcher>
#include <QStack>       //avt 11/29/19
#include <QSound>       //avt 12/10/19

#include "AudioDevice.hpp"
#include "commons.h"
#include "Radio.hpp"
#include "models/Modes.hpp"
#include "models/FrequencyList.hpp"
#include "Configuration.hpp"
#include "WSPRBandHopping.hpp"
#include "Transceiver.hpp"
#include "DisplayManual.hpp"
#include "psk_reporter.h"
#include "logbook/logbook.h"
#include "astro.h"
#include "MessageBox.hpp"
#include "NetworkAccessManager.hpp"
#include "logbook/WorkedBefore.hpp"    //avt 8/30/22

#define NUM_JT4_SYMBOLS 206                //(72+31)*2, embedded sync
#define NUM_JT65_SYMBOLS 126               //63 data + 63 sync
#define NUM_JT9_SYMBOLS 85                 //69 data + 16 sync
#define NUM_WSPR_SYMBOLS 162               //(50+31)*2, embedded sync
#define NUM_WSPR_LF_SYMBOLS 412            //300 data + 109 sync + 3 ramp
#define NUM_ISCAT_SYMBOLS 1291             //30*11025/256
#define NUM_MSK144_SYMBOLS 144             //s8 + d48 + s8 + d80
#define NUM_QRA64_SYMBOLS 84               //63 data + 21 sync
#define NUM_FT8_SYMBOLS 79
#define NUM_CW_SYMBOLS 250
#define TX_SAMPLE_RATE 48000
#define N_WIDGETS 37                        //avt 9/8/22
#define MAX_FAIL_COUNT 4                    //avt 9/11/22
#define MAX_CALL_QUEUE_AGE_MINUTES 15       //avt 10/13/22
#define MAX_PENDING_CALL_AGE_HOURS 1        //avt 10/13/22
#define MAX_CONF_CALL_AGE_HOURS 8          //avt 10/13/22
#define MAX_REPLY_QUEUE_SIZE 64             //avt 10/13/22
#define SHORT_FT8_TX_MSEC 11000             //avt 3/6/25
#define SHORT_JT65_TX_MSEC 44000            //avt 3/6/25
#define LATE_FT8_TX_MSEC 3000               //avt 3/6/25
#define LATE_JT65_TX_MSEC 6000              //avt 3/6/25
#define LATE_DEF_TX_MSEC 1000               //avt 3/6/25
#define DEF_MAX_TX_REPEAT 2                 //avt 3/27/25
#define QSO_FAIL_FNAME "qso_fails.txt"      //avt 3/27/25
#define POTA_LOG_FNAME "pota.txt"           //avt 3/27/25
#define INCR_LOG_FNAME "lotw.adi"           //avt 3/31/25
#define INCR_LOG_TMP_FNAME "lotw.adi.tmp"   //avt 3/31/25
#define COPY_LOG_SCRIPT "copy_log.sh"       //avt 4/4/25
#define FULL_LOG_FNAME "wsjtx_log.adi"      //avt 4/4/25
#define INCR_POTA_FNAME "pota.adi"          //avt 4/24/25
#define CONF_CALL_FNAME "conf_call.txt"
#define DEBUG_LOG_FNAME "debug.txt"

extern int volatile itone[NUM_ISCAT_SYMBOLS];   //Audio tones for all Tx symbols
extern int volatile icw[NUM_CW_SYMBOLS];	    //Dits for CW ID

//--------------------------------------------------------------- MainWindow
namespace Ui {
  class MainWindow;
}

class QSharedMemory;
class QSplashScreen;
class QSettings;
class QLineEdit;
class QFont;
class QHostInfo;
class EchoGraph;
class FastGraph;
class WideGraph;
class LogQSO;
class Transceiver;
class MessageAveraging;
class CabrilloLog;
class CabrilloLogWindow;
class ColorHighlighting;
class MessageClient;
class QTime;
class WSPRBandHopping;
class HelpTextWindow;
class WSPRNet;
class SoundOutput;
class Modulator;
class SoundInput;
class Detector;
class SampleDownloader;
class MultiSettings;
class EqualizationToolsDialog;
class DecodedText;

class MainWindow : public QMainWindow
{
  Q_OBJECT;

public:
  using Frequency = Radio::Frequency;
  using FrequencyDelta = Radio::FrequencyDelta;
  using Mode = Modes::Mode;

  explicit MainWindow(QDir const& temp_directory, bool multiple, MultiSettings *,
                      QSharedMemory *shdmem, unsigned downSampleFactor,
                      QSplashScreen *,
                      QWidget *parent = nullptr);
  ~MainWindow();

  int decoderBusy () const {return m_decoderBusy;}

public slots:
  void showSoundInError(const QString& errorMsg);
  void showSoundOutError(const QString& errorMsg);
  void showStatusMessage(const QString& statusMsg);
  void dataSink(qint64 frames);
  void fastSink(qint64 frames);
  void diskDat();
  void freezeDecode(int n);
  void guiUpdate();
  void doubleClickOnCall (Qt::KeyboardModifiers);
  void doubleClickOnCall2(Qt::KeyboardModifiers);
  void readFromStdout();
  void p1ReadFromStdout();
  void setXIT(int n, Frequency base = 0u);
  void setFreq4(int rxFreq, int txFreq);
  void msgAvgDecode2();
  void fastPick(int x0, int x1, int y);

protected:
  void keyPressEvent (QKeyEvent *) override;
  void closeEvent(QCloseEvent *) override;
  void childEvent(QChildEvent *) override;
  bool eventFilter(QObject *, QEvent *) override;

private slots:
  void initialize_fonts ();
  void on_tx1_editingFinished();
  void on_tx2_editingFinished();
  void on_tx3_editingFinished();
  void on_tx4_editingFinished();
  void on_tx5_currentTextChanged (QString const&);
  void on_tx6_editingFinished();
  void on_actionSettings_triggered();
  void on_monitorButton_clicked (bool);
  void on_actionAbout_triggered();
  void on_autoButton_clicked (bool);
  void on_stopTxButton_clicked();
  void on_ignoreCallButton_clicked();
  void on_addCallButton_clicked();
  void on_cqButton_clicked();
  void on_listenButton_clicked();
  void on_actionRelease_Notes_triggered ();
  void on_actionFT8_DXpedition_Mode_User_Guide_triggered();
  void on_actionQuick_Start_Guide_v2_triggered();
  void on_actionOnline_User_Guide_triggered();
  void on_actionLocal_User_Guide_triggered();
  void on_actionWide_Waterfall_triggered();
  void on_actionOpen_triggered();
  void on_actionOpen_next_in_directory_triggered();
  void on_actionDecode_remaining_files_in_directory_triggered();
  void on_actionDelete_all_wav_files_in_SaveDir_triggered();
  void on_actionOpen_log_directory_triggered ();
  void on_actionNone_triggered();
  void on_actionSave_all_triggered();
  void on_actionKeyboard_shortcuts_triggered();
  void on_actionSpecial_mouse_commands_triggered();
  void on_actionSolve_FreqCal_triggered();
  void on_actionCopyright_Notice_triggered();
  void on_DecodeButton_clicked (bool);
  void on_actionUpload_to_LOTW_triggered();   //avt 3/31/25
  void on_actionDownload_from_LOTW_triggered(); //avt 3/31/25
  void decode();
  void decodeBusy(bool b);
  void on_EraseButton_clicked();
  void band_activity_cleared ();
  void rx_frequency_activity_cleared ();
  void on_txFirstCheckBox_stateChanged(int arg1);
  void on_cbSkipGrid_clicked(bool checked);        //avt 2/12/19
  void on_cbSkipGrid_1_clicked(bool checked);        //avt 2/12/19
  void on_cbUseRR73_clicked(bool checked);        //avt 2/12/19
  void on_cbUseRR73_1_clicked(bool checked);        //avt 2/12/19
  void on_cbRndCqFreq_clicked(bool checked);      //avt 2/14/19
  void on_cbRndReplyFreq_clicked(bool checked);      //avt 2/23/19
  void on_cbAutoRestartCq_toggled(bool checked);    //avt 11/27/19
  void on_cbAutoListen_toggled(bool checked);     //avt 3/1/25
  void on_cbAlertDirected_toggled(bool checked);   //avt 8/30/22
  void on_cbAlertDxCq_toggled(bool checked);           //avt 9/20/22
  void on_cbAlertLocalCq_toggled(bool checked);           //avt 2/24/25
  void on_sbTxRepeat_valueChanged(int n);           //avt 4/26/25
  void on_cbConfStart_clicked(bool checked);     //avt 5/4/25
  void on_cbAddOther_clicked(bool checked);     //avt 5/5/25
  void on_cbAdd73orRR73_clicked(bool checked);     //avt 5/5/25
  void on_cbLockPeriod_clicked(bool checked);   //avt 5/6/25
  void set_dateTimeQSO(int m_ntx);
  void set_ntx(int n);
  void on_txrb1_toggled(bool status);
  void on_txrb1_doubleClicked ();
  void on_txrb2_toggled(bool status);
  void on_txrb3_toggled(bool status);
  void on_txrb4_toggled(bool status);
  void on_txrb4_doubleClicked ();
  void on_txrb5_toggled(bool status);
  void on_txrb5_doubleClicked ();
  void on_txrb6_toggled(bool status);
  void on_txb1_clicked();
  void on_txb1_doubleClicked ();
  void on_txb2_clicked();
  void on_txb3_clicked();
  void on_txb4_clicked();
  void on_txb4_doubleClicked ();
  void on_txb5_clicked();
  void on_txb5_doubleClicked ();
  void on_txb6_clicked();
  void on_lookupButton_clicked();
  void on_addButton_clicked();
  void on_dxCallEntry_textChanged (QString const&);
  void on_dxGridEntry_textChanged (QString const&);
  void on_dxCallEntry_returnPressed ();
  void on_genStdMsgsPushButton_clicked();
  void on_logQSOButton_clicked();
  void on_actionJT9_triggered();
  void on_actionJT65_triggered();
  void on_actionJT9_JT65_triggered();
  void on_actionJT4_triggered();
  void on_actionFT8_triggered();
  void on_TxFreqSpinBox_valueChanged(int arg1);
  void on_actionSave_decoded_triggered();
  void on_actionQuickDecode_toggled (bool);
  void on_actionMediumDecode_toggled (bool);
  void on_actionDeepestDecode_toggled (bool);
  void bumpFqso(int n);
  void on_actionErase_ALL_TXT_triggered();
  void on_reset_cabrillo_log_action_triggered ();
  void on_actionErase_wsjtx_log_adi_triggered();
  void on_actionExport_Cabrillo_log_triggered();
  void on_actionExport_POTA_log_triggered();    //avt 4/24/25
  void startTx2();
  void startP1();
  void stopTx();
  void stopTx2();
  void on_pbCallCQ_clicked();
  void on_pbAnswerCaller_clicked();
  void on_pbSendRRR_clicked();
  void on_pbAnswerCQ_clicked();
  void on_pbSendReport_clicked();
  void on_pbSend73_clicked();
  void on_rbGenMsg_clicked(bool checked);
  void on_rbFreeText_clicked(bool checked);
  void on_freeTextMsg_currentTextChanged (QString const&);
  void on_rptSpinBox_valueChanged(int n);
  void killFile();
  void on_tuneButton_clicked (bool);
  void on_pbR2T_clicked();
  void on_pbT2R_clicked();
  void acceptQSO (QDateTime const&, QString const& call, QString const& grid
                  , Frequency dial_freq, QString const& mode
                  , QString const& rpt_sent, QString const& rpt_received
                  , QString const& tx_power, QString const& comments
                  , QString const& name, QDateTime const& QSO_date_on, QString const& operator_call
                  , QString const& my_call, QString const& my_grid
                  , QString const& exchange_sent, QString const& exchange_rcvd
                  , QByteArray const& ADIF);
  void on_bandComboBox_currentIndexChanged (int index);
  void on_bandComboBox_activated (int index);
  void on_readFreq_clicked();
  void on_pbTxMode_clicked();
  void on_RxFreqSpinBox_valueChanged(int n);
  void on_outAttenuation_valueChanged (int);
  void rigOpen ();
  void handle_transceiver_update (Transceiver::TransceiverState const&);
  void handle_transceiver_failure (QString const& reason);
  void on_actionAstronomical_data_toggled (bool);
  void on_actionShort_list_of_add_on_prefixes_and_suffixes_triggered();
  void band_changed (Frequency);
  void monitor (bool);
  void stop_tuning ();
  void stopTuneATU();
  void auto_tx_mode(bool);
  void on_actionMessage_averaging_triggered();
  void on_contest_log_action_triggered ();
  void on_actionColors_triggered();
  void on_actionInclude_averaging_toggled (bool);
  void on_actionInclude_correlation_toggled (bool);
  void on_actionEnable_AP_DXcall_toggled (bool);
  void VHF_features_enabled(bool b);
  void on_sbSubmode_valueChanged(int n);
  void on_cbShMsgs_toggled(bool b);
  void on_cbSWL_toggled(bool b);
  void on_cbTx6_toggled(bool b);
  void on_cbMenus_toggled(bool b);
  void on_cbCQonly_toggled(bool b);
  void on_cbFirst_toggled(bool b);
  void on_cbAutoSeq_toggled(bool b);
  void on_cbHoldTxFreq_toggled(bool b); //avt 11/26/19
  void networkError (QString const&);
  void on_ClrAvgButton_clicked();
  void on_actionWSPR_triggered();
  void on_actionWSPR_LF_triggered();
  void on_syncSpinBox_valueChanged(int n);
  void on_TxPowerComboBox_currentIndexChanged(const QString &arg1);
  void on_sbTxPercent_valueChanged(int n);
  void on_cbUploadWSPR_Spots_toggled(bool b);
  void WSPR_config(bool b);
  void uploadSpots();
  void TxAgain();
  void uploadResponse(QString response);
  void on_WSPRfreqSpinBox_valueChanged(int n);
  void on_pbTxNext_clicked(bool b);
  void on_actionEcho_Graph_triggered();
  void on_actionEcho_triggered();
  void on_actionISCAT_triggered();
  void on_actionFast_Graph_triggered();
  void fast_decode_done();
  void on_actionMeasure_reference_spectrum_triggered();
  void on_actionErase_reference_spectrum_triggered();
  void on_actionMeasure_phase_response_triggered();
  void on_sbTR_valueChanged (int);
  void on_sbFtol_valueChanged (int);
  void on_cbFast9_clicked(bool b);
  void on_sbCQTxFreq_valueChanged(int n);
  void on_cbCQTx_toggled(bool b);
  void on_actionMSK144_triggered();
  void on_actionQRA64_triggered();
  void on_actionFreqCal_triggered();
  void splash_done ();
  void on_measure_check_box_stateChanged (int);
  void checkMSK144ContestType();
  int  setTxMsg(int n);
  bool stdCall(QString const& w);

private:
  Q_SIGNAL void initializeAudioOutputStream (QAudioDeviceInfo,
      unsigned channels, unsigned msBuffered) const;
  Q_SIGNAL void stopAudioOutputStream () const;
  Q_SIGNAL void startAudioInputStream (QAudioDeviceInfo const&,
      int framesPerBuffer, AudioDevice * sink,
      unsigned downSampleFactor, AudioDevice::Channel) const;
  Q_SIGNAL void suspendAudioInputStream () const;
  Q_SIGNAL void resumeAudioInputStream () const;
  Q_SIGNAL void startDetector (AudioDevice::Channel) const;
  Q_SIGNAL void FFTSize (unsigned) const;
  Q_SIGNAL void detectorClose () const;
  Q_SIGNAL void finished () const;
  Q_SIGNAL void transmitFrequency (double) const;
  Q_SIGNAL void endTransmitMessage (bool quick = false) const;
  Q_SIGNAL void tune (bool = true) const;
  Q_SIGNAL void sendMessage (unsigned symbolsLength, double framesPerSymbol,
      double frequency, double toneSpacing,
      SoundOutput *, AudioDevice::Channel = AudioDevice::Mono,
      bool synchronize = true, bool fastMode = false, double dBSNR = 99.,
                             int TRperiod=60) const;
  Q_SIGNAL void outAttenuationChanged (qreal) const;
  Q_SIGNAL void toggleShorthand () const;

private:
  void astroUpdate ();
  void writeAllTxt(QString message);
  void auto_sequence (DecodedText const& message);
  void hideMenus(bool b);
  void setColorHighlighting();
  
  void setCallInProg(QString);
  bool isDirDecode(DecodedText decodedText);         //avt 8/30/22
  bool isOtherDirDecode(DecodedText decodedText);       //avt 2/26/25
  bool isWantedDxLocalDecode(DecodedText decodedText);       //avt 2/26/25
  bool workedBefore(QString call, QString grid, bool isPota);  //avt 3/2/25
  bool isDx(QString call);  //avt 8/31/22
  QString continent(QString call);  //avt 3/2/25
  bool readPotaCalls();  //avt 8/31/22
  bool writePotaCalls();  //avt 8/31/22
  void updateModeCtrls();
  void incrFailCount(QString call);   //avt 9/11/22
  int failCount(QString call);        //avt 9/11/22
  bool readQsoFails ();               //avt 9/11/22
  bool writeQsoFails ();              //avt 9/11/22
  QString failMapToString(QMap<QString, int> map);  //avt 9/12/22
  QString mapToString(QMap<QString, QString> map);  //avt 9/9/22
  QString msgMapToString(QMap<QString, DecodedText> msgMap);  //avt 4/11/25
  void post_log(QString call);          //avt 10/9/22
  QString repliesSentMapToString(QMap<QString, QDateTime> map);   //avt 10/10/22
  void checkExpiredCallInfo();          //avt 10/13/22
  QString listToString(QList<DecodedText> list); //avt 10/13/22
  void setToCq();         //avt 3/7/25
  void flagCleanup();     //avt 2/28/25
  void selectLogMethod();  //avt 3/15/25
  void setAutoState (bool checked); //avt 3/9/25
  QList<DecodedText> splitFhMsg(QString msg_str);  //avt 3/18/25
  DecodedText decodeInSequence(DecodedText decodedText);    //avt 4/18/25
  void createCopyLogScript();   //avt 4/4/25
  void cleanupAdiBackupFiles();   //avt 4/4/25
  bool isAcceptableTxStartTime();
  bool isContest();   //avt 4/24/25
  bool isCabrilloLogEnabled();  //avt 5/2/25
  bool addPendingCall(DecodedText);     //avt 5/3/25
  bool canAdd73orRR73();    //avt 5/3/25
  bool canAddOtherMsg(DecodedText);    //avt 5/3/25
  void updateAddSkipButtons();    //avt 5/3/25
  void updateAutoButton();
  void updateAdd73orRR73();
  void updateTxPeriodLock();
  void clearAllCallInfo();
  void updateConfStart();
  void pendingCallAppend(DecodedText);
  void writeConfCall(QString, QString);
  bool readConfCalls (QString band);

  NetworkAccessManager m_network_manager;
  bool m_valid;
  QSplashScreen * m_splash;
  QString m_revision;
  bool m_multiple;
  MultiSettings * m_multi_settings;
  QPushButton * m_configurations_button;
  QSettings * m_settings;
  QScopedPointer<Ui::MainWindow> ui;

  LogBook m_logBook;            // must be before Configuration construction
  Configuration m_config;
  WSPRBandHopping m_WSPR_band_hopping;
  bool m_WSPR_tx_next;
  MessageBox m_rigErrorMessageBox;
  QScopedPointer<SampleDownloader> m_sampleDownloader;
  QScopedPointer<EqualizationToolsDialog> m_equalizationToolsDialog;

  QScopedPointer<WideGraph> m_wideGraph;
  QScopedPointer<EchoGraph> m_echoGraph;
  QScopedPointer<FastGraph> m_fastGraph;
  QScopedPointer<LogQSO> m_logDlg;
  QScopedPointer<Astro> m_astroWidget;
  QScopedPointer<HelpTextWindow> m_shortcuts;
  QScopedPointer<HelpTextWindow> m_prefixes;
  QScopedPointer<HelpTextWindow> m_mouseCmnds;
  QScopedPointer<MessageAveraging> m_msgAvgWidget;
  QScopedPointer<CabrilloLog> m_cabrilloLog;
  QScopedPointer<CabrilloLogWindow> m_contestLogWindow;
  QScopedPointer<ColorHighlighting> m_colorHighlighting;
  Transceiver::TransceiverState m_rigState;
  Frequency  m_lastDialFreq;
  QString m_lastBand;
  QString m_lastCallsign;
  Frequency  m_dialFreqRxWSPR;  // best guess at WSPR QRG

  Detector * m_detector;
  unsigned m_FFTSize;
  SoundInput * m_soundInput;
  Modulator * m_modulator;
  SoundOutput * m_soundOutput;
  QThread m_audioThread;

  qint64  m_msErase;
  qint64  m_secBandChanged;
  qint64  m_freqMoon;
  qint64  m_msec0;

  Frequency m_freqNominal;
  Frequency m_freqTxNominal;
  Astro::Correction m_astroCorrection;

  double  m_s6;
  double  m_tRemaining;

  float   m_DTtol;
  float   m_t0;
  float   m_t1;
  float   m_t0Pick;
  float   m_t1Pick;
  float   m_fCPUmskrtd;

  qint32  m_waterfallAvg;
  qint32  m_ntx;
  bool m_gen_message_is_cq;
  bool m_send_RR73;
  bool m_skipGrid;                //avt 2/12/19
  bool m_rndCqFreq;    //avt 2/14/19
  bool m_rndReplyFreq;    //avt 2/23/19
  qint32 m_prevCqFreq;  //avt 11/21/19
  QMap<QString, DecodedText> m_msgMap;    //avt 11/29/19
  QMap<QString, int> m_failMap;       //avt 9/11/22
  QSet<QString> m_loggedPotaCalls;    //avt 8/30/22
  QSet<QString> m_allPotaCalls;       //avt 8/30/22
  QMap<QString, QString> m_repliesRecd;        //avt 10/10/22 contains grid
  QMap<QString, QDateTime> m_repliesSent;      //avt 10/10/22 contains time
  QMap<QString, DecodedText> m_reportsRecd;    //avt 10/10/22 contains snr recd
  QMap<QString, QString> m_reportsSent;        //avt 10/10/22 contains snr sent
  QMap<QString, QString> m_exchsRecd;         //avt 5/2/25
  QMap<QString, QString> m_exchsSent;         //avt 5/2/25
  QList<DecodedText> m_pendingCallStack;     //avt 5/3/25
  QSet<QString> m_confCalls;          //avt 5/3/25
  QString m_msgAtStartTx;                      //avt 10/12/22
  QString m_msgAtEndTx;     //avt 3/18/25
  QSet<QString> m_loggedCalls;        //avt 8/21/22
  bool m_isTxPeriod;                  //avt 9/15/22
  qint32  m_timeout;
  qint32  m_XIT;
  qint32  m_setftx;
  qint32  m_ndepth;
  qint32  m_sec0;
  qint32  m_RxLog;
  qint32  m_nutc0;
  qint32  m_ntr;
  qint32  m_tx;
  qint32  m_hsym;
  qint32  m_TRperiod;
  qint32  m_nsps;
  qint32  m_hsymStop;
  qint32  m_inGain;
  qint32  m_ncw;
  qint32  m_secID;
  qint32  m_idleMinutes;
  qint32  m_nSubMode;
  qint32  m_nclearave;
  qint32  m_minSync;
  qint32  m_dBm;
  qint32  m_pctx;
  qint32  m_nseq;
  qint32  m_nWSPRdecodes;
  qint32  m_k0;
  qint32  m_kdone;
  qint32  m_nPick;
  FrequencyList_v2::const_iterator m_frequency_list_fcal_iter;
  qint32  m_nTx73;
  qint32  m_UTCdisk;
  qint32  m_wait;
  qint32  m_isort;
  qint32  m_max_dB;
  qint32  m_nDXped=0;
  qint32  m_Nlist=12;
  qint32  m_Nslots=5;
  qint32  m_tAutoOn;
  qint32  m_maxStrikes=3;      //Max # of repeats: 3 strikes and you're out

  bool    m_btxok;		//True if OK to transmit
  bool    m_diskData;
  bool    m_loopall;
  bool    m_decoderBusy;
  bool    m_txFirst;
  bool    m_auto;
  bool    m_restart;
  bool    m_startAnother;
  bool    m_saveDecoded;
  bool    m_saveAll;
  bool    m_widebandDecode;
  bool    m_call3Modified;
  bool    m_dataAvailable;
  bool    m_bDecoded;
  bool    m_noSuffix;
  bool    m_blankLine;
  bool    m_decodedText2;
  bool    m_freeText;
  bool    m_sentFirst73;
  int     m_currentMessageType;
  QString m_currentMessage;
  int     m_lastMessageType;
  QString m_lastMessageSent;
  bool    m_bShMsgs;
  bool    m_bSWL;
  bool    m_uploadSpots;
  bool    m_uploading;
  bool    m_txNext;
  bool    m_grid6;
  bool    m_tuneup;
  bool    m_bTxTime;
  bool    m_rxDone;
  bool    m_bSimplex; // not using split even if it is available
  bool    m_bEchoTxOK;
  bool    m_bTransmittedEcho;
  bool    m_bEchoTxed;
  bool    m_bFastMode;
  bool    m_bFast9;
  bool    m_bFastDecodeCalled;
  bool    m_bDoubleClickAfterCQnnn;
  bool    m_bRefSpec;
  bool    m_bClearRefSpec;
  bool    m_bTrain;
  bool    m_bUseRef;
  bool    m_bFastDone;
  bool    m_bAltV;
  bool    m_bNoMoreFiles;
  bool    m_bDoubleClicked;
  bool    m_bCallingCQ;
  bool    m_bAutoReply;
  bool    m_bCheckedContest;
  bool    m_bWarnedSplit=false;
  bool    m_bTUmsg;
  bool    m_bLastDecodeIsCQ;    //avt 2/23/19
  int     m_dupTxCount;         //avt 8/20/22
  bool    m_cleanup;      //avt 2/28/25
  QString m_supportedAutoModes; //avt 3/1/25
  bool    m_enableSound;    //avt 3/3/25
  QDateTime m_txStartDt;    //avt 3/6/25
  QString m_callInProg;     //avt 2/6/25
  bool    m_newAutoQso;     //avt 3/11/25
  bool    m_logbookRead;   //avt 3/18/25
  bool    m_alertSounded;   //avt 3/18/25
  bool    m_addCallSounded;    //avt 4/4/25
  bool    m_setupStable;    //avt 3/27/25
  int     m_prefTxLimit;    //avt 4/3/25
  int     m_incrLogCount;   //avt 4/4/25
  QString m_lastDecodeDisp;   //avt 4/17/25
  bool    m_confReplied;

  enum
    {
      CALLING,
      REPLYING,
      REPORT,
      ROGER_REPORT,
      ROGERS,
      SIGNOFF
    }
    m_QSOProgress;        //State machine counter
    
  enum {CALL, GRID, DXCC, MULT};

  int			m_ihsym;
  int			m_nzap;
  int			m_npts8;
  float		m_px;
  float   m_pxmax;
  float		m_df3;
  int			m_iptt0;
  bool		m_btxok0;
  int			m_nsendingsh;
  double	m_onAirFreq0;
  bool		m_first_error;

  char    m_msg[100][80];

  // labels in status bar
  QLabel tx_status_label;
  QLabel config_label;
  QLabel mode_label;
  QLabel last_tx_label;
  QLabel next_tx_label;
  QLabel auto_tx_label;
  QLabel band_hopping_label;
  QProgressBar progressBar;
  QLabel watchdog_label;

  QFuture<void> m_wav_future;
  QFutureWatcher<void> m_wav_future_watcher;
  QFutureWatcher<void> watcher3;
  QFutureWatcher<QString> m_saveWAVWatcher;

  QProcess proc_jt9;
  QProcess p1;
  QProcess p3;
  QProcess *upLotw;    //avt 4/1/25
  QProcess *dnLotw;    //avt 4/1/25

  WSPRNet *wsprNet;

  QTimer m_guiTimer;
  QTimer ptt1Timer;                 //StartTx delay
  QTimer ptt0Timer;                 //StopTx delay
  QTimer logQSOTimer;
  QTimer killFileTimer;
  QTimer tuneButtonTimer;
  QTimer uploadTimer;
  QTimer tuneATU_Timer;
  QTimer TxAgainTimer;
  QTimer minuteTimer;
  QTimer splashTimer;
  QTimer p1Timer;
  QTimer cleanupTimer;        //avt 10/13/22
  QTimer replyCallTimer;

  QString m_path;
  QString m_baseCall;
  QString m_hisCall;
  QString m_hisGrid;
  QString m_appDir;
  QString m_palette;
  QString m_dateTime;
  QString m_mode;
  QString m_modeTx;
  QString m_fnameWE;            // save path without extension
  QString m_rpt;
  QString m_nextRpt;
  QString m_rptSent;
  QString m_rptRcvd;
  QString m_qsoStart;
  QString m_qsoStop;
  QString m_cmnd;
  QString m_cmndP1;
  QString m_msgSent0;
  QString m_fileToSave;
  QString m_calls;
  QString m_CQtype;
  QString m_opCall;
  QString m_fm0;
  QString m_fm1;
  QString m_xSent;               //Contest exchange sent
  QString m_xRcvd;               //Contest exchange received
  QString m_currentBand;
  QString m_nextCall;
  QString m_nextGrid;

  QSet<QString> m_pfx;
  QList<QString> m_copy_log;      //avt 4/4/25
  QSet<QString> m_sfx;

  QDateTime m_dateTimeQSOOn;
  QDateTime m_dateTimeLastTX;

  QSharedMemory *mem_jt9;
  QString m_QSOText;
  unsigned m_msAudioOutputBuffered;
  unsigned m_framesAudioInputBuffered;
  unsigned m_downSampleFactor;
  QThread::Priority m_audioThreadPriority;
  bool m_bandEdited;
  bool m_splitMode;
  bool m_monitoring;
  bool m_tx_when_ready;
  bool m_transmitting;
  bool m_tune;
  bool m_tx_watchdog;           // true when watchdog triggered
  bool m_block_pwr_tooltip;
  bool m_PwrBandSetOK;
  bool m_bVHFwarned;
  bool m_bDisplayedOnce;
  Frequency m_lastMonitoredFrequency;
  double m_toneSpacing;
  int m_firstDecode;
  QProgressDialog m_optimizingProgress;
  QTimer m_heartbeat;
  MessageClient * m_messageClient;
  PSK_Reporter *psk_Reporter;
  DisplayManual m_manual;
  QHash<QString, QVariant> m_pwrBandTxMemory; // Remembers power level by band
  QHash<QString, QVariant> m_pwrBandTuneMemory; // Remembers power level by band for tuning
  QByteArray m_geometryNoControls;
  QVector<double> m_phaseEqCoefficients;

  //---------------------------------------------------- private functions
  void readSettings();
  void set_application_font (QFont const&);
  void setDecodedTextFont (QFont const&);
  void writeSettings();
  QString modeString();
  void debugToFile(QString str);    //avt 11/25/19
  bool isAutoCqMode();   //avt 12/8/19
  bool isAutoListenMode();      //avt 3/1/25
  bool isAnyAutoMode();      //avt 3/5/25
  bool isDirCqEnabled(); //avt 3/12/20
  bool isDxCqEnabled();       //avt 9/20/22
  bool isLocalCqEnabled();       //avt 1/24/25
  DecodedText peekMessage();    //avt 4/11/25
  void removeMessage(QString call);    //avt 9/17/22
  void showQueue();         //avt 1/5/20
  QString setToString(QSet<QString> set);   //avt 8/31/22
  void removeCallsByPeriod();   //avt 3/28/25
  void testDecodedText();     //avt 4/8/25
  bool isCorrectPeriod(DecodedText);
  bool isDecodeFromLastPeriod(DecodedText);
  void createStatusBar();
  void updateStatusBar();
  void genStdMsgs(QString rpt, bool unconditional = false);
  void genCQMsg();
  void clearDX ();
  void lookup();
  void ba2msg(QByteArray ba, char* message);
  void msgtype(QString t, QLineEdit* tx);
  void stub();
  void statusChanged();
  void fixStop();
  bool shortList(QString callsign);
  void transmit (double snr = 99.);
  void rigFailure (QString const& reason);
  void pskSetLocal ();
  void pskPost(DecodedText const& decodedtext);
  void displayDialFrequency ();
  void transmitDisplay (bool);
  void processMessage(DecodedText const&, Qt::KeyboardModifiers = 0);
  void replyToCQ (QTime, qint32 snr, float delta_time, quint32 delta_frequency, QString const& mode, QString const& message_text, bool low_confidence, quint8 modifiers);
  void locationChange(QString const& location);
  void replayDecodes ();
  void postDecode (bool is_new, QString const& message);
  void postWSPRDecode (bool is_new, QStringList message_parts);
  void enable_DXCC_entity (bool on);
  void switch_mode (Mode);
  void WSPR_scheduling ();
  void freqCalStep();
  void setRig (Frequency = 0);  // zero frequency means no change
  void WSPR_history(Frequency dialFreq, int ndecodes);
  QString WSPR_hhmm(int n);
  void fast_config(bool b);
  void CQTxFreq();
  void useNextCall();
  void abortQSO();
  bool isWorked(int itype, QString key, float fMHz=0, QString="");

  QString save_wave_file (QString const& name
                          , short const * data
                          , int seconds
                          , QString const& my_callsign
                          , QString const& my_grid
                          , QString const& mode
                          , qint32 sub_mode
                          , Frequency frequency
                          , QString const& his_call
                          , QString const& his_grid) const;
  
  

  void read_wav_file (QString const& fname);
  void decodeDone ();
  void subProcessFailed (QProcess *, int exit_code, QProcess::ExitStatus);
  void subProcessError (QProcess *, QProcess::ProcessError);
  void uploadLotwFinished(int, QProcess::ExitStatus);      //avt 4/1/25
  void downloadLotwFinished(int, QProcess::ExitStatus);      //avt 4/1/25
  void replyCallTimeout();
  void lotwError (QProcess *, QProcess::ProcessError);
  void statusUpdate () const;
  void update_watchdog_label ();
  void on_the_minute ();
  void add_child_to_event_filter (QObject *);
  void remove_child_from_event_filter (QObject *);
  void setup_status_bar (bool vhf);
  void tx_watchdog (bool triggered);
  qint64  nWidgets(QString t);
  void displayWidgets(qint64 n);
  void vhfWarning();
  QChar current_submode () const; // returns QChar {0} if sub mode is
                                  // not appropriate
  void write_transmit_entry (QString const& file_name);
  void setIncrLogCount();     //avt 4/6/25
  void clearFailCount(QString);   //avt 4/6/25
};

extern int killbyname(const char* progName);
extern void getDev(int* numDevices,char hostAPI_DeviceName[][50],
                   int minChan[], int maxChan[],
                   int minSpeed[], int maxSpeed[]);
extern int next_tx_state(int pctx);

#endif // MAINWINDOW_H
