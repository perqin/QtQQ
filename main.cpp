#include <QtGui/QApplication>
#include <QtDeclarative>
#include "qmlapplicationviewer.h"
#include "qqstars.h"
#include "myimage.h"
#include "mysvgview.h"
#include "utility.h"
#include "myhttprequest.h"
#include "downloadimage.h"
#include "mynetworkaccessmanagerfactory.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

#if defined(Q_WS_SIMULATOR)
    /*QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::HttpProxy);
    proxy.setHostName("localhost");
    proxy.setPort(8888);
    QNetworkProxy::setApplicationProxy(proxy);*/
#endif

    qmlRegisterType<QQCommand>("QQItemInfo", 1,0, "QQ");
    qmlRegisterType<FriendInfo>("QQItemInfo", 1,0, "FriendInfo");
    qmlRegisterType<GroupInfo>("QQItemInfo", 1,0, "GroupInfo");
    qmlRegisterType<DiscuInfo>("QQItemInfo", 1,0, "DiscuInfo");
    qmlRegisterType<QQItemInfo>("QQItemInfo", 1,0, "QQItemInfo");
    qmlRegisterType<ChatMessageInfo>("QQItemInfo", 1, 0, "ChatMessageInfo");
    qmlRegisterType<ChatMessageInfoList>("QQItemInfo", 1, 0, "ChatMessageInfoList");
    qmlRegisterType<MyImage>("mywidgets", 1,0, "MyImage");
    qmlRegisterType<MySvgView>("mywidgets", 1, 0, "SvgView");

    QmlApplicationViewer viewer;
    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);

    QDeclarativeEngine *engine = viewer.engine();
    engine->setNetworkAccessManagerFactory (new MyNetworkAccessManagerFactory());//给qml设置网络请求所用的类

    Utility *utility=Utility::createUtilityClass ();
    QNetworkRequest* request = utility->getHttpRequest ()->getNetworkRequest ();
    request->setRawHeader ("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002&callback=1&id=2");//和腾讯服务器打交道需要设置这个
    request->setHeader (QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    request = utility->getDownloadImage ()->getHttpRequest ()->getNetworkRequest ();
    request->setRawHeader ("Referer", "http://web2.qq.com/webqq.html");//需要设置这个，不然腾讯服务器不响应你的请求
    request->setRawHeader ("Accept", "image/webp,*/*;q=0.8");

    utility->initUtility (new QSettings, engine);

    QDeclarativeComponent component0(engine, "./qml/Api/QQApi.qml");
    QQCommand *qqapi = qobject_cast<QQCommand *>(component0.create ());
    engine->rootContext ()->setContextProperty ("myqq", qqapi);



    viewer.setMainQmlFile(QLatin1String("qml/symbian1/main.qml"));
    viewer.showExpanded();

    return app.exec();
}
