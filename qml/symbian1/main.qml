import QtQuick 1.0
import QQItemInfo 1.0
import com.nokia.symbian 1.0
import com.nokia.extras 1.0
import "Component"
import "../js/main.js" as Script

PageStackWindow {
    id: app;

    ToolBarLayout {
        id: mainTools;
        ToolButtonWithTip {
            id: messageButton;
            toolTipText: qsTr("Message");
            iconSource: "../pics/tem.svg";
            onClicked: {
                signalCenter.showMessage(qsTr("Press again to quit"));
                //pageStack.replace(Qt.resolvedUrl(".qml"));
            }
            /*Bubble {
                anchors.verticalCenter: parent.top;
                anchors.horizontalCenter: parent.horizontalCenter;
                text: {
                    var n = 0;
                    if (tbsettings.remindAtme) n += infoCenter.atme;
                    if (tbsettings.remindPletter) n += infoCenter.pletter;
                    if (tbsettings.remindReplyme) n += infoCenter.replyme;
                    return n>0 ? n : "";
                }
                visible: text != "";
            }*/
        }
        ToolButtonWithTip {
            toolTipText: qsTr("Contact");
            iconSource: "../pics/tem.svg";
            //onClicked: pageStack.push(Qt.resolvedUrl(".qml"));
        }
        ToolButtonWithTip {
            toolTipText: qsTr("Apps");
            iconSource: "../pics/tem.svg";
            onClicked: {
                if (pageStack.currentPage.objectName != "AppsPage")
                    pageStack.push(Qt.resolvedUrl("Apps/AppsPage.qml"));
            }
        }
        ToolButtonWithTip {
            toolTipText: qsTr("Menu");
            iconSource: "toolbar-menu";
            //onClicked: pageStack.push(Qt.resolvedUrl("MorePage.qml"));
        }
    }

    initialPage: MainPage {
        id: mainPage;
    }

    SettingsCenter {
        id: settingsCenter;
    }

    SignalCenter {
        id: signalCenter;
    }

    Constant {
        id: constant;
    }

    InfoBanner {
        id: infoBanner;
        iconSource: "../pics/error.svg";
        //platformInverted: tbsettings.whiteTheme;
    }

    ToolTip {
        id: toolTip;
        //platformInverted: tbsettings.whiteTheme;
        visible: false;
    }
/*
    showStatusBar: true;

    StatusPaneText { id: statusPaneText; }

    InfoCenter { id: infoCenter; }

    AudioWrapper { id: audioWrapper; }

    WorkerScript {
        id: worker;
        property bool running: false;
        source: "../js/WorkerScript.js";
        onMessage: running = messageObject.running;
    }

    ImageUploader {
        id: imageUploader;
        property variant caller: null;
        function signForm(params){
            return Script.BaiduRequest.generateSignature(params);
        }
        function jsonParse(data){
            return JSON.parse(data);
        }
        uploader: HttpUploader {}
        onUploadFinished: signalCenter.imageUploadFinished(caller, result);
    }

    HttpUploader {
        id: uploader;
        property variant caller: null;
        onUploadStateChanged: Script.uploadStateChanged();
    }

    AudioRecorder {
        id: recorder;
        outputLocation: utility.tempPath+"/audio.amr";
    }



*/
    //Component.onCompleted: Script.initialize(signalCenter, tbsettings, utility, worker, uploader, imageUploader);
    Component.onCompleted: Script.initialize(signalCenter, settingsCenter);
}

/*
Rectangle {
    width: 360
    height: 640

    function testQQFinished(error, data) {//服务器返回qq是否需要验证码
        if(error){//如果出错了
            myqq.checkAccount(testQQFinished)
            return
        }

        if( myqq.loginStatus == QQ.Logining ){//如果登录状态为登录中
            var temp = data.split("'")
            var uin = temp[5]//测试qq是否需要验证码后返回的uin值（密码加密中需要用到）
            if( temp[1]=="0" ){
                utility.consoleLog("检测是否需要验证码结束："+data)
                myqq.login(uin, temp[3])//不需要验证码，直接登录
            }else{
                utility.consoleLog("需要输入验证码")
                var url = "https://ssl.captcha.qq.com/getimage?aid=1003903&r=0.9101365606766194&uin="+myqq.userQQ+"&cap_cd="+uin
                image_code.source = url
                /*此处需要去获取验证码*//*
            }
        }
    }

    Image{
        id: image_code
        anchors.bottom: field_account.top
        anchors.bottomMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
    }

    TextField{
        id: field_account
        anchors.centerIn: parent
        implicitWidth: parent.width*2/3
    }
    TextField{
        id: field_password
        anchors.horizontalCenter: field_account.horizontalCenter
        anchors.top: field_account.bottom
        anchors.topMargin: 10
        implicitWidth: field_account.implicitWidth
    }

    Button{
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: field_password.bottom
        anchors.topMargin: 20
        text: "    登  录    "
        onClicked: {
            if(field_account.text==""||field_password.text=="")
                return

            myqq.userQQ = field_account.text
            //设置userqq之后，c++端会自动将配置和缓存目录改到此用户下，所以设置myqq.userQQ要谨慎，一半是确定用户要登录此账户后才设置，如按下登录按钮
            myqq.userPassword = field_password.text
            myqq.loginStatus = QQ.Logining//登录状态设置为登录中
            myqq.checkAccount(testQQFinished)//先检测qq号是否需要验证码
        }
    }
}
*/
