#include "qqstars.h"
#include "utility.h"
#include <QSettings>
#include <QScriptEngine>
#include "myhttprequest.h"
#include "mynetworkaccessmanagerfactory.h"

QQCommand *QQCommand::firstQQCommand = NULL;
QQCommand *QQCommand::getFirstQQCommand()
{
    return firstQQCommand;
}

QQCommand::QQCommand( QObject *parent) :
    FriendInfo(parent)
{
    if(firstQQCommand==NULL)
        firstQQCommand = this;

    connect (this, SIGNAL(stateChanged(States)), SLOT(onStateChanged()));
    
    Utility *utility=Utility::createUtilityClass ();
    int temp1 = utility->value ("proxyType", QNetworkProxy::NoProxy).toInt ();
    QString temp2 = utility->value ("proxyLocation", "").toString ();
    QString temp3 = utility->value ("proxyPort", "").toString ();
    QString temp4 = utility->value ("proxyUsername", "").toString ();
    QString temp5 = utility->value ("proxyPassword", "").toString ();
    utility->setApplicationProxy (temp1, temp2, temp3, temp4, temp5);
    
    setUserQQ (utility->value ("mainqq","").toString ());
    m_loginStatus = WaitLogin;//当前为离线(还未登录)
    chatImageID = 0;//初始化为0
    
    request = new QNetworkRequest();
    request->setUrl (QUrl("http://d.web2.qq.com/channel/poll2"));
    request->setRawHeader ("Origin", "http://d.web2.qq.com");
    request->setRawHeader ("Accept", "*/*");
    request->setRawHeader ("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002&callback=1&id=2");
    request->setRawHeader ("Content-Type", "application/x-www-form-urlencoded");
    request->setRawHeader ("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.66 Safari/537.36 LBBROWSER");
    
    manager = new NetworkAccessManager(this);
    connect (manager, SIGNAL(finished(QNetworkReply*)), SLOT(poll2Finished(QNetworkReply*)));
    
    jsEngine = new QScriptEngine();//此对象用来加载js文件（为qq提供api）
    loadApi ();//加载api的js文件
    
    reply = NULL;
    poll2Timerout_count=0;//记录网络请求的连续超时次数
    poll2Error_count=0;//记录网络请求连续出错的次数
    connect (utility, SIGNAL(networkOnlineStateChanged(bool)), SLOT(onNetworkOnlineStateChanged(bool)));
    poll2_timer = new QTimer(this);
    poll2_timer->setSingleShot (true);//设置为单发射器
    connect (poll2_timer, SIGNAL(timeout()), SLOT(onPoll2Timeout()));
    abortPoll_timer = new QTimer(this);
    abortPoll_timer->setSingleShot (true);//设置为单发射器
    
    http_image = new MyHttpRequest(this);//此网络请求对象专门用来获取聊天中收到图片的真实下载地址
    http_image->getNetworkRequest ()->setRawHeader (
                "Referer", "http://d.web2.qq.com/proxy.html?v=20110331002&callback=1&id=2");//必须设置，不然请求腾讯的数据会返回出错
}

QQCommand::LoginStatus QQCommand::loginStatus() const
{
    return m_loginStatus;
}

QString QQCommand::userPassword() const
{
    return m_userPassword;
}

QString QQCommand::userQQ() const
{
    return m_userQQ;
}

bool QQCommand::rememberPassword() const
{
    if(isCanUseSetting())
        return mysettings->value ("rememberPassword", false).toBool ();
    return false;
}

bool QQCommand::autoLogin() const
{
    if(isCanUseSetting())
        return mysettings->value ("autoLogin", false).toBool ();
    return false;
}

void QQCommand::beginPoll2()
{
    disconnect (abortPoll_timer, SIGNAL(timeout()), reply, SIGNAL(aboutToClose()));
    //qDebug()<<"reply1:"<<reply;
    reply=manager->post (*request, poll2_data);
    //qDebug()<<"reply2:"<<reply;
    connect (abortPoll_timer, SIGNAL(timeout()), reply, SIGNAL(aboutToClose()));
    poll2_timer->start (100000);//网络请求超时定时器
}

void QQCommand::poll2Finished(QNetworkReply *replys)
{
    poll2_timer->stop ();//停止计算请求是否超时的计时器
    /*if(replys->error ()==QNetworkReply::NoError) {
        QByteArray array = replys->readAll ();
        emit poll2ReData (array);
        QJsonParseError json_error;
        QJsonDocument document = QJsonDocument::fromJson (array, &json_error);
        if(json_error.error == QJsonParseError::NoError) 
        {
            if( document.isObject () ){
                QJsonObject obj = document.object ();
                if( obj["retcode"].isDouble () ){
                    int retcode = obj["retcode"].toInt ();
                    if( retcode==0 ){
                        QJsonArray arr = obj["result"].toArray ();
                        foreach (QJsonValue temp, arr) {
                            obj = temp.toObject ();
                            QString poll_type=obj["poll_type"].toString ();
                            obj = obj["value"].toObject ();
                            if( poll_type=="message" ){
                                disposeFriendMessage (obj);//解析好友的普通消息
                            }else if( poll_type=="input_notify" ){
                                disposeFriendMessage (obj, InputNotify);//解析好友正在输入的消息
                            }else if( poll_type=="buddies_status_change" ){
                                disposeFriendStatusChanged(obj);//好友状态改变信息
                            }else if( poll_type=="group_message" ){
                                disposeGroupMessage (obj);//解析群消息
                            }else if( poll_type=="discu_message" ){
                                disposeDiscuMessage (obj);//解析讨论组消息
                            }else if( poll_type=="file_message" ){
                                //qDebug()<<"发送文件消息";
                                disposeFriendMessage (obj, FileMessage);
                            }else if( poll_type=="av_request" ){
                                //qDebug()<<"视频聊天消息";
                                disposeFriendMessage (obj, AvRequest);
                            }else if( poll_type=="av_refuse" ){
                                //qDebug()<<"取消开视频";
                                disposeFriendMessage (obj, AvRefuse);
                            }else if( poll_type=="shake_message" ){
                                //qDebug()<<"窗口抖动消息";
                                disposeFriendMessage (obj, ShakeWindow);//解析窗口抖动消息
                            }else if( poll_type=="system_message" ){
                                disposeSystemMessage (obj);//解析系统消息
                            }else if( poll_type=="sys_g_msg" ){
                                disposeSystemMessage (obj);//解析系统消息
                            }else if(poll_type == "sess_message"){
                                //disposeStrangerMessage (obj);//解析陌生人的消息
                                disposeFriendMessage (obj);//解析好友的普通消息
                            }else{
                                qDebug()<<"QQCommand:其他消息"<<poll_type;
                            }
                        }
                        beginPoll2();
                    }else if(retcode==102){
                        beginPoll2();
                    }else if(retcode==116){
                        beginPoll2();
                    }else{
                        qDebug()<<"QQCommand:qq已掉线，即将重新登录";
                        QMetaObject::invokeMethod (this, "reLogin");//调用槽reLogin（在qml中定义）
                    }
                }
            }else if(document.isArray ()){
                QJsonArray arr = document.array ();
                qDebug()<<"QQCommand:心跳包收到的是一个数组"<<arr.count ();
            }
        }
    }else if(replys->error ()!=QNetworkReply::OperationCanceledError){//如果不是手动取消的
        ++poll2Error_count;
        if(poll2Error_count>1){
            qDebug()<<"QQCommand:网络请求连续出错"<<poll2Error_count<<"次，将重新登录";
            poll2Error_count=0;
            QMetaObject::invokeMethod (this, "reLogin");//调用槽reLogin重新登录（在qml中定义）
        }else
            beginPoll2();//重新post
    }*/
}

void QQCommand::initUserPassword()
{
    if(isCanUseSetting()){
        QString pass=mysettings->value ("password", "").toString ();
        setUserPassword (Utility::createUtilityClass ()->stringUncrypt (pass, "xingchenQQ"));
    }
}

void QQCommand::onSettingsChanged()
{
    emit rememberPasswordChanged ();
    emit autoLoginChanged ();
    initUserPassword ();
    setState ((States)mysettings->value ("myState", (int)Online).toInt ());//设置自己的状态
}

void QQCommand::onStateChanged()
{
    mysettings->setValue ("myState", (int)state());
}

void QQCommand::onPoll2Timeout()
{
    ++poll2Timerout_count;//记录网络请求的连续超时次数
    qDebug()<<"QQCommand:网络请求超时："<<poll2Timerout_count;
    if(reply){
        reply->abort ();//取消网络请求
    }
    beginPoll2 ();//再次开始网络请求
}

void QQCommand::onNetworkOnlineStateChanged(bool isOnline)
{
    qDebug()<<"QQCommand:网络在线状态改变为："<<isOnline;
    if(isOnline){
        abortPoll_timer->stop ();//停止计时器
        if(!reply||!reply->isRunning ()){//如果心跳包没有在进行
            qDebug()<<"QQCommand:由于断网影响，将重新登录qq";
            QMetaObject::invokeMethod (this, "reLogin");//调用槽reLogin重新登录（在qml中定义）
        }
    }else{
        abortPoll_timer->start (60000);//启动取消心跳包的定时器
    }
}

void QQCommand::downImageFinished(bool error, const QString &path, const QString &name)
{
    QStringList list = name.split ("_");//分割字符串
    QString rootUin=list[0];//图片发送者本身或者图片发送者所在群的uin
    int messageID = list[1].toInt ();//分离出messageID
    int imageID = list[2].toInt ();//分离出imageID
    
    QString senderType;
    QRegExp reg("[A-Z][a-z]+_[0-9]+");//符合条件的字符串，例如Friend_826169080
    if(reg.indexIn (path)>=0){//如果找到了
        senderType = reg.cap (0);
        QStringList list = senderType.split ("_");
        senderType=list[0];//图片所有者的类型（如果是好友发送的就是Friend，如果是群成员发送的就是Group）
    }else{
        qDebug()<<"QQCommand:从下载路径中提取uin和消息发送者类型出错";
        return;
    }
    QQItemInfo* info = createQQItemInfo (rootUin, senderType);
    //获取图片所有者的信息
    if(error){//如果出错
        qDebug()<<"QQCommand:图片"<<name<<"下载出错";
        QString image_url = getImageUrlById (imageID);//通过图片id获得储存的图片真实下载地址
        QString save_name = name.mid (0, name.size ()-4);//去除末尾的图片后缀才是要保存的图片名字
        Utility::createUtilityClass ()->//重新去下载图片
                downloadImage (this, SLOT(downImageFinished(bool,QString,QString)), 
                               QUrl(image_url), path, save_name);//重新下载图片
        return;
    }
    ChatMessageInfo* message_info = info->getChatMessageInfoById (messageID);
    //通过图片所在消息的id获取储存消息信息的对象
    QString content = message_info->contentData ();//获得消息储存的内容
    QString old_img = "<img src=\"qrc:/images/loading.png\" imageID="+QString::number (imageID)+">";//旧的img标签中的内容
    QString new_img = "<img src=\"file:///"+path+name+"\">";//新img标签内容
    content.replace (old_img, new_img);//将old_img标签替换为新的内容
    message_info->setContentData (content);//替换消息内容，qml端会自动刷新消息
}

void QQCommand::getImageUrlFinished(QNetworkReply *replys)
{
    ImageInfo image_info = queue_imageInfo.dequeue ();//从列队中取出队首
    if(replys->error() == QNetworkReply::NoError)//如果网络请求没有出错
    {
        QString image_url = replys->rawHeader ("Location");//从返回的http头部中取出图片的真实下载地址
        if(image_url==""){//如果真实下载地址为空
            qDebug()<<"QQCommand:获取图片"<<image_info.imageID<<"的真实下载地址失败，网络返回内容为："<<replys->readAll ();
            return;
        }
        const QQItemInfo* root_info = image_info.messageInfo->getParent ();
        //获取图片所有者信息的对象（如果图片为好友发送，那就是那个好友的info，如果是群，那就是群的ifno）
        QString save_path = root_info->localCachePath ()+"/cacheImage/";//获取图片的缓存路径
        QString save_name = root_info->uin ()+"_"+
                QString::number (image_info.messageInfo->messageId ())+"_"+
                QString::number (image_info.imageID)+"_"+
                QDateTime::currentDateTime ().toString (Qt::ISODate).replace (QRegExp("\\D"),"");//设置要保存的图片名（不加后缀）
        setImageUrlById (image_info.imageID, image_url);//将图片id和真实下载链接的对应关系储存起来
        Utility::createUtilityClass ()->//启动网络请求获取图片
                downloadImage (this, SLOT(downImageFinished(bool,QString,QString)), 
                               QUrl(image_url), save_path, save_name);
    }else{//如果获取图片的真实下载地址出错
        queue_imageInfo<<image_info;//不要忘了把图片信息重新加到列队
        http_image->get (this, SLOT(getImageUrlFinished(QNetworkReply*)), image_info.filePath);
        //重新去获取图片的真实下载地址
    }
}

void QQCommand::loadApi()
{
    QString fileName = "qml/Api/api.js";
    QFile scriptFile(fileName);
    if (!scriptFile.open(QIODevice::ReadOnly))
        qDebug()<<QString::fromUtf8("QQCommand:打开")+fileName+QString::fromUtf8("失败")<<scriptFile.errorString();
    QString contents = scriptFile.readAll ();
    scriptFile.close();
    jsEngine->evaluate(contents, fileName);
}

/*QString QQCommand::disposeMessage(QJsonObject &obj, ChatMessageInfo* message_info)
{
    QString result;
    FontStyle font_style;
    QJsonArray content = obj["content"].toArray ();
    QJsonValue temp2 = content[0];
    if(temp2.isArray ()){
        QJsonArray font = temp2.toArray ();
        foreach (QJsonValue temp3, font) {
            if(temp3.isObject ()){
                obj = temp3.toObject ();
                font_style.size = 3;//obj["size"].toInt ();
                font_style.color = "black";//obj["color"].toString ();
                //QJsonArray style = obj["style"].toArray ();
                font_style.bold = false;//(bool)style[0].toInt ();//加黑
                font_style.italic = false;//(bool)style[1].toInt ();//斜体
                font_style.underline = false;//(bool)style[2].toInt ();//下划线
                font_style.family = "新宋体";//obj["name"].toString ();
            }
        }
    }
    for( int i=1;i<content.size ();++i ){
        temp2 = content[i];
        if(temp2.isArray ()){
            QJsonArray array = temp2.toArray ();
            QString array_name = array[0].toString ();
            if(array_name=="cface"){//为群图片消息
                foreach (QJsonValue temp3, array) {
                    if(temp3.isObject ()){
                        obj = temp3.toObject ();
                        QString file_id = doubleToString (obj, "file_id");
                        //QString key = obj["key"].toString ();
                        QString name = obj["name"].toString ();
                        name.replace ("{", "%7B");
                        name.replace ("}", "%7D");
                        QString server = obj["server"].toString ();
                        QStringList tem_list = server.split (":");
                        server = tem_list[0];
                        QString port = tem_list[1];
                        
                        GroupInfo* root_info = qobject_cast<GroupInfo*>(const_cast<QQItemInfo*>(message_info->getParent ()));
                        if(root_info==NULL){
                            qDebug()<<"QQCommand:将消息对象从QQItemInfo转换为GroupInfo失败";
                            result.append (textToHtml (font_style, "[图片加载失败...]"));
                        }else{
                            QString file_path = "http://web2.qq.com/cgi-bin/get_group_pic?type=0&gid="+
                                    root_info->code ()+"&uin="+message_info->senderUin ()+
                                    "&rip="+server+"&rport="+port+"&fid="+file_id+
                                    "&pic="+name+"&vfwebqq="+property ("vfwebqq").toString ();
                            result.append (disposeImageMessage(message_info, file_path));
                        }
                    }
                }
            }else if(array_name=="offpic"){//为图片消息
                foreach (QJsonValue temp3, array) {
                    if(temp3.isObject ()){
                        obj = temp3.toObject ();
                        QString file_path = obj["file_path"].toString ();
                        file_path.replace ("/", "%2F");
                        file_path = "http://d.web2.qq.com/channel/get_offpic2?file_path="+file_path+"&f_uin="+message_info->senderUin ()
                                +"&clientid="+property ("clientid").toString ()
                                +"&psessionid="+property ("psessionid").toString ();
                        result.append (disposeImageMessage(message_info, file_path));
                        //处理图片消息，并将处理的结果添加到result
                    }
                }
            }else if(array_name=="face"){//为表情消息
                QString faceName = QString::number (array[1].toInt ());//转化为int
                QString pngFace_number = "21 25 32 33 34 36 39 42 45 50 59 64 85 86 91 124";
                if(pngFace_number.indexOf (faceName)>=0)
                    faceName.append (".png");
                else
                    faceName.append (".gif");
                QString data = "<img widht=\"25\" height=\"25\" src=\"./faces/classic/"+faceName+"\">";
                //qDebug()<<data;
                result.append (data);//添加纯文本消息
                //qDebug()<<"表情消息,"<<"表情代码："<<array[1].toInt ();
                //data.append (QString("{")+"\"type\":"+QString::number (Face)+",\"face_code\":"+QString::number (array[1].toInt ())+"},");
            }else{
                qDebug()<<"其他类型的数据："<<array_name;
            }
        }else if(temp2.isString ()&&temp2.toString ()!=""){//否则为纯文本消息
            QString textFace_str = "双喜 鞭炮 灯笼 发财 K歌 购物 邮件 帅 喝彩 祈祷 爆筋 棒棒糖 喝奶 下面 香蕉 飞机 开车 高铁左车头 车厢 高铁右车头 多云 下雨 钞票 熊猫 灯泡 风车 闹钟 打伞 彩球 钻戒 沙发 纸巾 药 手枪 青蛙";
            QString content = temp2.toString ();
            QRegExp reg("\\[.+\\]");
            QStringList list = content.split (reg);
            reg.indexIn (content);

            for (int i=0;i<list.length ();++i) {
                QString str = list[i];
                if(str!=""){
                    str = textToHtml (font_style, str);
                    result.append (str);
                }
                if(i<=reg.captureCount ()){
                    str = reg.cap (i);
                    str.replace ("[","");
                    str.replace ("]","");
                    if(str=="") break;
                    if(str!=" "&&textFace_str.indexOf (str)>=0){//如果是表情代码
                        str = "<img widht=\"25\" height=\"25\" src=\"qrc:/faces/classic/"+str+".png\">";
                    }else{
                        str = textToHtml (font_style, "["+str+"]");
                    }
                    result.append (str);
                }
            }
        }
    }
    return result;
}

void QQCommand::disposeFriendStatusChanged(QJsonObject &obj)
{
    QString uin = doubleToString (obj, "uin");
    QString status = obj["status"].toString ();
    createFriendInfo (uin)->setStateToString (status);//设置好友状态
}

void QQCommand::disposeFriendMessage(QJsonObject &obj, QQCommand::MessageType type)
{
    //qDebug()<<"是聊天消息";
    QString from_uin = doubleToString (obj, "from_uin");
    //int msg_id = obj.value ("msg_id").toInt ();
    //int msg_id2 = obj.value ("msg_id2").toInt ();
    //QString msg_type = doubleToString (obj, "msg_type");
    //QString reply_ip = doubleToString (obj, "reply_ip");
    //QString to_uin = doubleToString (obj, "to_uin");

    switch (type) 
    {
    case GeneralMessage:{
        QQItemInfo *info = createQQItemInfo (from_uin, QQItemInfo::Friend);//先获取消息发送者的信息
        int msg_id = info->getMessageIndex();//为新消息获取一个id
        
        ChatMessageInfo *message_info = info->getChatMessageInfoById (msg_id);
        //通过消息id获得一个储存消息各种信息的对象
        message_info->setSenderUin (from_uin);//将消息的信息存进去
        message_info->setDate (QDate::currentDate ());//将消息发送日期存进去
        message_info->setTime (QTime::currentTime ());//将消息发送时间存进去
        QString data = disposeMessage (obj, message_info);//处理这条消息的内容（一定要放在SenderUin之后）
        message_info->setContentData (data);//将处理后的内容设置给消息对象
        
        info->addChatRecord (message_info);//将消息加到发送者info对象中
        
        //qDebug()<<"收到了好友消息："<<data;
        emit newMessage (from_uin, (int)QQItemInfo::Friend, message_info);//发送信号告诉qml有新的消息
        break;
    }
    case InputNotify:
        emit friendInputNotify (from_uin);//发送好友正在输入的信号
        break;
    case FileMessage:{
        QString mode = obj["mode"].toString ();
        if( mode=="recv" ){
            QString file_name = obj["name"].toString ();
            emit messageArrive (Friend, from_uin, "{\"content\":[{\"type\":"+QString::number (FileMessage)+",\"flag\":1,\"name\":\""+file_name+"\"}]}");
        }else{
            emit messageArrive (Friend, from_uin, "{\"content\":[{\"type\":"+QString::number (FileMessage)+",\"flag\":0}]}");
        }
        break;
    }
    case AvRequest:
        break;
    case AvRefuse:
        break;
    case ShakeWindow://如果是窗口抖动的消息
        addChatPage (from_uin, QQItemInfo::Friend);//现将此聊天窗口显示出来
        emit shakeWindow (from_uin);//然后发送信号告诉窗口有人抖动他
        break;
    default:
        break;
    }
}

void QQCommand::disposeGroupMessage(QJsonObject &obj, QQCommand::MessageType type)
{
    //qDebug()<<"是群消息";
    QString from_uin = doubleToString (obj, "from_uin");
    //QString group_code = doubleToString (obj, "group_code");
    //int msg_id = obj.value ("msg_id").toInt ();
    //int msg_id2 = obj.value ("msg_id2").toInt ();
    //QString msg_type = doubleToString (obj, "msg_type");
    //QString reply_ip = doubleToString (obj, "reply_ip");
    //QString to_uin = doubleToString (obj, "to_uin");
    QString send_uin = doubleToString (obj, "send_uin");//发送者的uin

    switch (type) {
    case GeneralMessage:{
        QQItemInfo *info = createQQItemInfo (from_uin, QQItemInfo::Group);//先获取储存群信息的对象
        int msg_id = info->getMessageIndex();//同好友消息
        
        ChatMessageInfo *message_info = info->getChatMessageInfoById (msg_id);
        message_info->setSenderUin (send_uin);//注意！！！此处的发送者uin不是群的uin，而是将消息发到群的成员的uin
        message_info->setDate (QDate::currentDate ());
        message_info->setTime (QTime::currentTime ());
        QString data = disposeMessage (obj, message_info);//先处理消息内容
        message_info->setContentData (data);
        
        info->addChatRecord (message_info);//给from_uin的info对象增加聊天记录
        emit newMessage (from_uin, (int)QQItemInfo::Group, message_info);
        break;
    }
    default:
        break;
    }
    
}

void QQCommand::disposeDiscuMessage(QJsonObject &obj, QQCommand::MessageType type)
{
    //qDebug()<<"是讨论组消息";
    //QString from_uin = doubleToString (obj, "from_uin");;
    QString did = doubleToString (obj, "did");
    //int msg_id = obj.value ("msg_id").toInt ();
    //int msg_id2 = obj.value ("msg_id2").toInt ();
    //QString msg_type = doubleToString (obj, "msg_type");
    //QString reply_ip = doubleToString (obj, "reply_ip");
    //QString to_uin = doubleToString (obj, "to_uin");
    QString send_uin = doubleToString (obj, "send_uin");
    
    switch (type) {
    case GeneralMessage:{
        QQItemInfo *info = createQQItemInfo (did, QQItemInfo::Discu);
        int msg_id = info->getMessageIndex();
        
        ChatMessageInfo *message_info = info->getChatMessageInfoById (msg_id);
        message_info->setSenderUin (send_uin);//注意！！！此处的发送者uin不是群的uin，而是将消息发到讨论组的成员的uin
        message_info->setDate (QDate::currentDate ());
        message_info->setTime (QTime::currentTime ());
        QString data = disposeMessage (obj, message_info);//先处理基本消息
        message_info->setContentData (data);
        
        info->addChatRecord (message_info);//给from_uin的info对象增加聊天记录
        emit newMessage (did, (int)QQItemInfo::Discu, message_info);
        break;
    }
    default:
        break;
    }
}

void QQCommand::disposeStrangerMessage(QJsonObject &, QQCommand::MessageType )
{
    QString from_uin = doubleToString (obj, "from_uin");
    QString msg_id = doubleToString (obj, "msg_id");
    QString msg_id2 = doubleToString (obj, "msg_id2");
    QString msg_type = doubleToString (obj, "msg_type");
    QString reply_ip = doubleToString (obj, "reply_ip");
    QString to_uin = doubleToString (obj, "to_uin");
    
}

void QQCommand::disposeSystemMessage(QJsonObject &obj)
{
    QString type = obj["type"].toString ();
    if(type == "verify_required"){//好友验证信息
        QString account = doubleToString (obj, "account");
        QString from_uin = doubleToString (obj, "from_uin");
        //emit messageArrive (SystemMessage, from_uin, "{\"type\":"+QString::number (FriendVerify)+",\"account\"\":"+account+"\"}");
    }else if(type == "group_admin_op"){//管理员变动信息
        QString from_uin = doubleToString (obj, "from_uin");
        QString uin = doubleToString (obj, "uin");
        QString uin_flag = doubleToString (obj, "uin_flag");
        //emit messageArrive (SystemMessage, from_uin, "{\"type\":"+QString::number (GroupAdmin)+",\"uin\":\""+uin+"\",\"flag\":\""+uin_flag+"\"}");
    }else if(type == "group_leave"){//群成员变动信息
        QString from_uin = doubleToString (obj, "from_uin");
        QString old_member = doubleToString (obj, "old_member");
        //emit messageArrive (SystemMessage, from_uin, "{\"type\":"+QString::number (GroupLeave)+",\"old_member\":\""+old_member+"\"}");
    }else{//其他系统消息
        qDebug()<<"其他系统消息:"<<type;
    }
}

void QQCommand::disposeFileMessage(QJsonObject &obj)
{
    QString from_uin = doubleToString (obj, "from_uin");
    QString mode = obj["mode"].toString ();
    if( mode=="recv" ){
        QString file_name = obj["name"].toString ();
        emit messageArrive (Friend, from_uin, "{\"content\":[{\"type\":"+QString::number (SendFile)+"}, \"name\":\""+file_name+"\"]}");
    }else{
        emit messageArrive (Friend, from_uin, "{\"content\":[{\"type\":"+QString::number (SendFile)+"}]}");
    }
}

void QQCommand::disposeAvMessage(QJsonObject &obj, bool open)
{
    QString from_uin = doubleToString (obj, "from_uin");
    emit messageArrive (Friend, from_uin, "{\"content\":[{\"type\":"+QString::number (open?AvRequest:AvRefuse)+"}]}");
}

void QQCommand::disposeShakeMessage(QJsonObject &obj)
{
    QString from_uin = doubleToString (obj, "from_uin");
    emit messageArrive (Friend, from_uin, "{\"content\":[{\"type\":"+QString::number (ShakeWindow)+"}]}");
}

QString QQCommand::doubleToString(QJsonObject &obj, const QString &name)
{
    if(!obj.isEmpty ()){
        QJsonValue temp = obj[name];
        if(temp.isDouble ())
            return QString::number ((quint64)obj[name].toDouble ());
    }
    return name;
}*/

QString QQCommand::textToHtml(QQCommand::FontStyle &style, QString data)
{
    data.replace("&","&amp;");     
    data.replace(">","&gt;");
    data.replace("<","&lt;");
    data.replace("\"","&quot;");
    data.replace("\'","&#39;");
    data.replace(" ","&nbsp;");
    data.replace("\n","<br>");
    data.replace("\r","<br>");
    //上面这几行代码的顺序不能乱，否则会造成多次替换
    
    QString result="<font";
    if(style.size>0)
        result.append (" size=\""+QString::number (style.size)+"\"");
    if(style.color!=""){
        if(style.color[0].isNumber ())
            result.append (" color=\"#"+style.color+"\"");
        else
            result.append (" color=\""+style.color+"\"");
    }
    if(style.family!="")
        result.append (" face=\""+style.family+"\"");
    result.append (">");
    if(style.bold)
        result.append ("<b>");
    if(style.underline)
        result.append ("<u>");
    if(style.italic)
        result.append ("<i>");
    result.append (data);//把文本包含进去
    if(style.italic)
        result.append ("</i>");
    if(style.underline)
        result.append ("</u>");
    if(style.bold)
        result.append ("</b>");
    result.append ("</font>");
    
    return result;
}

QQItemInfo *QQCommand::createQQItemInfo(const QString& uin, const QString& typeString)
{
    if(uin==""||typeString==""){
        qDebug()<<"QQCommand-createQQItemInfo:参数不合法,uin:"<<uin<<"type:"+typeString;
        return NULL;
    }
    
    QString name = typeString+uin;
    if(map_itemInfo.value (name, NULL)){
        QQItemInfo* info = qobject_cast<QQItemInfo*>(map_itemInfo[name]);
        return info;
    }
    QDeclarativeEngine *engine = Utility::createUtilityClass ()->qmlEngine ();
    QDeclarativeComponent component(engine, "./qml/QQItemInfo/"+typeString+"Info.qml");
    QQItemInfo* info = qobject_cast<QQItemInfo*>(component.create ());
    if(info!=NULL){
        map_itemInfo[name] = info;
        info->setParent (this);
        info->setUserQQ (userQQ());
        info->setUin (uin);
    }
    return info;
}

void QQCommand::setLoginStatus(QQCommand::LoginStatus arg)
{
    if (m_loginStatus != arg) {
        m_loginStatus = arg;
        if(arg == WaitLogin){//如果登录状态变为离线
            poll2_timer->stop ();//停止计算请求是否超时的计时器
            reply->abort ();//停止心跳包
            //closeChatWindow();//关闭和好友聊天的窗口
            clearQQItemInfos();//清空所有的好友信息
            chatImageID = 0;//chatImageID回到缺省值
            map_imageUrl.clear ();//情况image的id和url值对

        }else if(arg == LoginFinished){//如果登录完成

        }
        emit loginStatusChanged();
    }
}

void QQCommand::startPoll2(const QByteArray &data)
{
    poll2_data = data;
    //poll2_timer.start ();
    beginPoll2();
}

void QQCommand::setUserQQ(QString arg)
{
    if (m_userQQ != arg) {
        m_userQQ = arg;
        FriendInfo::setUserQQ (arg);
        FriendInfo::setAccount (arg);
        FriendInfo::setUin (arg);
        emit userQQChanged();
    }
}

void QQCommand::setUserPassword(QString arg)
{
    if (m_userPassword != arg) {
        //qDebug()<<"设置了密码"<<arg;
        m_userPassword = arg;
        emit userPasswordChanged();
    }
}

void QQCommand::downloadImage(int senderType, QUrl url, QString account, QString imageSize, QScriptValue callbackFun)
{
    QString path = QQItemInfo::localCachePath ((QQItemInfo::QQItemType)senderType, userQQ(), account);
    //先获取此qq为account，类型为senderType的缓存目录，将此目录传给下载图片的函数。此图片下载完成就会存入此路径
    Utility::createUtilityClass ()->downloadImage (callbackFun, url, path, "avatar-"+imageSize);
}

FriendInfo* QQCommand::createFriendInfo(const QString uin)
{
    FriendInfo* info = qobject_cast<FriendInfo*>(createQQItemInfo(uin, QQItemInfo::Friend));
    return info;
}

GroupInfo* QQCommand::createGroupInfo(const QString uin)
{
    GroupInfo* info = qobject_cast<GroupInfo*>(createQQItemInfo(uin, QQItemInfo::Group));
    return info;
}

DiscuInfo* QQCommand::createDiscuInfo(const QString uin)
{
    DiscuInfo* info = qobject_cast<DiscuInfo*>(createQQItemInfo(uin, QQItemInfo::Discu));
    return info;
}

void QQCommand::addChatPage(QString uin, int senderType)
{
    /*if(uin==""||senderType<0)
        return;
    QString typeStr = QQItemInfo::typeToString ((QQItemInfo::QQItemType)senderType);//获取此类型的字符串表达形式
    
    if(map_chatPage.contains(typeStr+uin)){//如果已经存在
        emit activeChatPageChanged (map_chatPage[typeStr+uin]);//活跃的聊天Page改变为temp
        mainChatWindowCommand->show ();//显示出聊天窗口
        return;//如果已经处在此Page就返回
    }
    
    QQmlEngine *engine = Utility::createUtilityClass ()->qmlEngine ();
    if(mainChatWindowCommand.isNull ()){
        QQmlComponent component(engine, "./qml/Chat/ChatWindowCommand.qml");
        QObject *temp_obj = component.create ();
        //qDebug()<<"创建窗口是否出错："<<component.errorString ();
        mainChatWindowCommand = qobject_cast<MyWindow*>(temp_obj);
        if(mainChatWindowCommand){
            connect (mainChatWindowCommand.data (), &MyWindow::closeing, this, &QQCommand::onChatMainWindowClose);
            //链接信号和槽，为聊天主窗口关闭时销毁对象所用
            foreach (QQuickItem *item, mainChatWindowCommand->contentItem ()->childItems ()) {
                if(item->objectName () == "ChatWindowCommandItem"){
                    mainChatWindowCommand_item = item;//将聊天页面的父对象储存起来
                    break;
                }
            }
        }else{
            qDebug()<<"创建ChatWindowCommand.qml出错";
            return;//如果出错就返回
        }
    }

    QString qmlName = "./qml/Chat/"+typeStr+"ChatPage.qml";
    QQmlComponent component(engine, qmlName);
    QQuickItem *item = qobject_cast<QQuickItem*>(component.create ());//新建聊天页面
    if(item&&mainChatWindowCommand_item){
        item->setParentItem (mainChatWindowCommand_item);//设置聊天页面的父对象
        item->setProperty ("myuin", uin);//设置他的uin
        item->setProperty ("type", senderType);//设置他的类型
        map_chatPage[typeStr+uin] = item;//储存聊天页面
        QQItemInfo* item_info = createQQItemInfo (uin, typeStr);
        if(item_info==NULL){//如果对象为空就返回
            qDebug()<<"QQCommand-addChatPage:创建QQItemInfo对象失败";
            return;
        }
        emit addChatPageToWindow (item);//发送信号告知qml增加了聊天页
    }else{
        qDebug()<<"创建"+qmlName+"出错";
    }
    mainChatWindowCommand->show ();//显示出聊天窗口*/
}

void QQCommand::removeChatPage(QString uin, int senderType)
{
    /*QQItemInfo::QQItemType type = (QQItemInfo::QQItemType)senderType;
    QString typeStr = QQItemInfo::typeToString (type);//获取此类型的字符串表达形式
    QQuickItem *item = map_chatPage.value (typeStr+uin, NULL);
    if(item!=NULL){
        item->deleteLater ();//销毁此对象
    }else{
        qDebug()<<typeStr+uin<<"page已经为NULL";
    }
    map_chatPage.remove (typeStr+uin);//移除此对象
    //qDebug()<<item;
    foreach (QQuickItem *temp, map_chatPage) {//改变当前活跃页面为首先找到的第一个不为空的chatPage
        if(temp){
            //qDebug()<<temp;
            emit activeChatPageChanged (temp);
            break;
        }else{
            QString key = map_chatPage.key (temp);
            map_chatPage.remove (key);//如果对象已经为空则移除此对象
            qDebug()<<key+"为NULL，已被销毁";
        }
    }*/
}

QVariant QQCommand::value(const QString &key, const QVariant &defaultValue) const
{
    return mysettings->value (key, defaultValue);
}

void QQCommand::setValue(const QString &key, const QVariant &value)
{
    mysettings->setValue (key, value);
}

void QQCommand::openSqlDatabase()
{
    FriendInfo::openSqlDatabase (userQQ());//打开数据库
}

QString QQCommand::getMovieImageFrameCachePath()
{
    return QDir::homePath ()+"/webqq";
}

/*void QQCommand::saveAlias(int type, QString uin, QString alias)
{
    QString name = QQItemInfo::typeToString ((QQItemInfo::QQItemType)type)+uin;
    map_alias[name] = alias;
}*/

void QQCommand::updataApi(const QString& content)
{
    qDebug()<<"更新api.js"<<content;
}

QQItemInfo *QQCommand::createQQItemInfo(const QString& uin, QQItemInfo::QQItemType type)
{
    QString typeString = QQItemInfo::typeToString (type);
    return createQQItemInfo (uin, typeString);
}

void QQCommand::clearQQItemInfos()
{
    foreach (QQItemInfo* info, map_itemInfo) {
        if(info!=NULL)
            info->deleteLater ();
    }
    map_itemInfo.clear ();
}

QString QQCommand::disposeImageMessage(ChatMessageInfo* message_info, QString image_url)
{
    int image_id = getImageIndex();//为这个图片获得一个唯一的id
    ImageInfo image_info;//为图片创建一个储存自己信息的结构体对象
    image_info.filePath=image_url;//储存能获取图片真实下载地址的字符串
    image_info.imageID=image_id;//储存图片的id
    image_info.messageInfo=message_info;//储存图片所在消息的对象的指针
    queue_imageInfo<<image_info;//将图片的信息加入队列
    http_image->get (this, SLOT(getImageUrlFinished(QNetworkReply*)), image_url);
    //进行网络请求,获取图片的真实下载地址
    return "<img src=\"qrc:/images/loading.png\" imageID="+QString::number (image_id)+">";
    //返回此字符串，用于img标签的占位，等图片下载完成会替换此img标签
}

int QQCommand::getImageIndex()
{
    return chatImageID++;
}

QString QQCommand::getImageUrlById(int image_id)
{
    return map_imageUrl[image_id];
}

void QQCommand::setImageUrlById(int image_id, const QString &url)
{
    //qDebug()<<"将id为："<<image_id<<"图片的下载地址设置为："<<url;
    map_imageUrl[image_id]=url;
}

bool QQCommand::isChatPageExist(const QString& uin, int senderType)
{
    QString typeStr = QQItemInfo::typeToString ((QQItemInfo::QQItemType)senderType);//获取此类型的字符串表达形式
    return map_chatPage.contains(typeStr+uin);
}

void QQCommand::addFriendUin(const QString &uin)
{
    friendsUin.append (uin+" ");
    //qDebug()<<"增加好友uin:"+uin<<friendsUin;
}

bool QQCommand::isStranger(const QString &uin)
{
    return friendsUin.indexOf (uin)<0;
}

QString QQCommand::getHash()
{
    QScriptValueList list;
    list<<QScriptValue(userQQ())<<QScriptValue(Utility::createUtilityClass ()->getCookie ("ptwebqq"));
    return jsEngine->globalObject ().property ("getHash").call (QScriptValue(), list).toString ();
}

QString QQCommand::encryptionPassword(const QString &uin, const QString &code)
{
    qDebug()<<QString::fromUtf8("QQCommand:调用了加密密码的函数");
    QScriptValueList list;
    list<<QScriptValue(userPassword())<<QScriptValue(uin)<<QScriptValue(code);
    QString result = jsEngine->globalObject ().property ("encryptionPassword").call (QScriptValue(), list).toString ();
    qDebug()<<QString::fromUtf8("加密后的密码为：")<<result;
    return result;
}

QString QQCommand::getLoginedQQInfo()
{
    Utility *utility = Utility::createUtilityClass ();
    QByteArray reply="[";
    QString qqs = utility->value ("qq_account", "").toString ();
    QStringList qq_list = qqs.split (",");
    foreach (QString qq, qq_list) {
        if(qq!=""){
            QStringList temp = qq.split (".");
            if(temp.size ()==2){//如果有两个，一个为qq号，一个为昵称
                FriendInfo info;
                QString account = temp[0];
                info.setUserQQ (account);
                info.setAccount (account);
                reply.append ("{\"account\":\""+account
                              +"\",\"nick\":\""+QByteArray::fromHex (temp[1].toUtf8 ())
                        +"\",\"avatarSource\":\""+info.avatar240 ()+"\"},");
            }
        }
    }
    reply.replace (reply.size ()-1,1,"]");
    return reply;
}

void QQCommand::removeLoginedQQInfo(const QString account, bool rmLocalCache)
{
    Utility *utility = Utility::createUtilityClass ();
    QString qqs = utility->value ("qq_account", "").toString ();
    QStringList qq_list = qqs.split (",");
    foreach (QString qq, qq_list) {
        if(qq!=""){
            QStringList temp = qq.split (".");
            if(temp.size ()==2){//如果有两个，一个为qq号，一个为昵称
                if(temp[0]==account){//如果查找到此qq
                    qqs.replace (qq+",", "");//替换掉
                    utility->setValue ("qq_account", qqs);//替换掉原来的值
                    FriendInfo info;
                    info.setUserQQ (account);
                    info.setAccount (account);
                    info.clearSettings ();//清除配置内容
                    if(rmLocalCache){//如果要删除本地缓存
                        utility->removePath (info.localCachePath ());
                    }
                    return;
                }
            }
        }
    }
}

void QQCommand::addLoginedQQInfo(const QString account, const QString nick)
{
    Utility *utility = Utility::createUtilityClass ();
    QString qqs = utility->value ("qq_account", "").toString ();
    QString addStr = account+"."+nick.toUtf8 ().toHex ()+",";
    if(qqs.indexOf (addStr)<0){//如果这条信息不存在
        qqs.insert (0, addStr);
        utility->setValue ("qq_account", qqs);//添加进去
    }
}

void QQCommand::setRememberPassword(bool arg)
{
    if (mysettings&&rememberPassword ()!= arg) {
        mysettings->setValue ("rememberPassword", arg);
        if(!arg)
            mysettings->remove ("password");
        emit rememberPasswordChanged();
    }
}

void QQCommand::setAutoLogin(bool arg)
{
    if (mysettings&&autoLogin() != arg) {
        mysettings->setValue ("autoLogin", arg);
        emit autoLoginChanged();
    }
}

void QQCommand::saveUserPassword()
{
    if(rememberPassword()&&mysettings){//先判断是否记住了密码
        QString pass = Utility::createUtilityClass ()->stringEncrypt (userPassword (), "xingchenQQ");
        mysettings->setValue ("password", pass);
    }
}
