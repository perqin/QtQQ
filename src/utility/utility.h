#ifndef UTILITY_H
#define UTILITY_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QTimer>
#include <QPoint>
#include <QSettings>
#include <QNetworkConfigurationManager>
#include <QDeclarativeEngine>
#include <QPointer>

class UtilityPrivate : public QObject
{
    Q_OBJECT
    Q_ENUMS(ProxyType)
public:
    enum ProxyType {
        DefaultProxy,
        Socks5Proxy,
        NoProxy,
        HttpProxy,
        HttpCachingProxy,
        FtpCachingProxy
    };
};

class MyHttpRequest;
class DownloadImage;
class Utility : public QObject
{
    Q_OBJECT
public:
    static Utility *createUtilityClass();
private:
    explicit Utility(QObject *parent = 0);
    QPointer<QDeclarativeEngine> engine;
    QPointer<QSettings> mysettings;
    
    MyHttpRequest *http_request;
    DownloadImage *download_image;

    QNetworkConfigurationManager networkConfigurationManager;
    
    char numToStr(int num);//将数字按一定的规律换算成字母
    QByteArray strZoarium(const QByteArray &str);//按一定的规律加密字符串(只包含数字和字母的字符串)
    QByteArray unStrZoarium(const QByteArray &str);//按一定的规律解密字符串(只包含数字和字母的字符串)
    QByteArray fillContent(const QByteArray &str, int length);//将字符串填充到一定的长度
private slots:

public:
    Q_INVOKABLE void consoleLog(QString str);//输出调试信息
    Q_INVOKABLE QString getCookie( QString cookieName );
    QDeclarativeEngine *qmlEngine();
    MyHttpRequest *getHttpRequest();
    DownloadImage *getDownloadImage();
    bool networkIsOnline() const;
signals:
    void networkOnlineStateChanged(bool isOnline);
public slots:
    void initUtility(QSettings *settings=0, QDeclarativeEngine *qmlEngine=0);
    void setQmlEngine( QDeclarativeEngine *new_engine );
    QPoint mouseDesktopPos();
    
    void setQSettings(QSettings *settings);
    void setValue( const QString & key, const QVariant & value);
    QVariant value(const QString & key, const QVariant & defaultValue = QVariant()) const;
    void removeValue( const QString & key );
    
    void downloadImage( QScriptValue callbackFun, QUrl url, QString savePath, QString saveName );
    void downloadImage( QObject *caller, QByteArray slotName, QUrl url, QString savePath, QString saveName );
    void httpGet(QObject *caller, QByteArray slotName, QUrl url, bool highRequest=false);
    void httpPost(QObject *caller, QByteArray slotName, QUrl url, QByteArray data, bool highRequest=false);
    void httpGet(QScriptValue callbackFun, QUrl url, bool highRequest=false );
    void httpPost(QScriptValue callbackFun, QUrl url, QByteArray data="", bool highRequest=false );
    void socketAbort();
    void setApplicationProxy( int type, QString location, QString port, QString username, QString password );
    
    QString stringEncrypt(const QString &content, QString key);//加密任意字符串，中文请使用utf-8编码
    QString stringUncrypt(const QString &content_hex, QString key);//解密加密后的字符串
    
    void removePath(QString dirPath ,bool deleteHidden = true, bool deleteSelf = true );
};

#endif // UTILITY_H
