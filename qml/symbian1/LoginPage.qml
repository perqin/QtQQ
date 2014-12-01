import QtQuick 1.0
import com.nokia.symbian 1.0
import "Component"
//import "../js/main.js" as Script

MyPage {
    id: page;

    //property bool forceLogin: false;

    objectName: "LoginPage";

    title: qsTr("Login");

    tools: ToolBarLayout {
        ToolButtonWithTip {
            toolTipText: qsTr("Quit");
            iconSource: "toolbar-back";
            onClicked: Qt.quit();
        }
        ToolButtonWithTip {
            toolTipText: qsTr("Setting");
            iconSource: "toolbar-settings";
            onClicked: ;
        }
    }

    Column {
        anchors.left: parent.left; anchors.leftMargin: constant.paddingLarge;
        anchors.right: parent.right; anchors.rightMargin: constant.paddingLarge;
        anchors.top: parent.top; anchors.topMargin: 120;
        spacing: constant.paddingXLarge;
        Item {
            id: userRow;
            width: parent.width; height: userField.height;
            Image {
                id: userIcon;
                source: "../pics/user.png";
                height: userField.height; width: userField.height;
                anchors.left: parent.left;
            }
            TextField {
                id: userField;
                anchors.left: userIcon.right; anchors.leftMargin: constant.paddingLarge;
                anchors.right: parent.right;
            }
        }
        Item {
            id: passRow;
            width: parent.width; height: passField.height;
            Image {
                id: passIcon;
                source: "../pics/user.png";
                height: passField.height; width: passField.height;
                anchors.left: parent.left;
            }
            TextField {
                id: passField;
                anchors.left: passIcon.right; anchors.leftMargin: constant.paddingLarge;
                anchors.right: parent.right;
            }
        }
        ToolButton {
            text: qsTr("Login");
            anchors.right: parent.right;
            onClicked: {
                //for test ui
                pageStack.pop();
            }
        }
        Item {
            id: checkRow;
            width: parent.width; height: rememberPassCheck.height;
            CheckBox {
                id: rememberPassCheck;
                text: qsTr("Remember password");
                anchors.left: parent.left;
            }
            CheckBox {
                id: autoLoginCheck;
                text: qsTr("Auto login");
                anchors.left: parent.left; anchors.leftMargin: parent.width / 2;
            }
        }
    }
    Column {
        anchors.right: parent.right; anchors.rightMargin: constant.paddingLarge;
        anchors.bottom: parent.bottom; anchors.bottomMargin: constant.paddingMedium;
        spacing: constant.paddingSmall;
        Text {
            font: constant.labelFont;
            color: constant.colorLight;
            text: "<a href=\"link\">%1</a>".arg(qsTr("Forget password"));
            //onLinkActivated: signalCenter.openBrowser("https://aq.qq.com/cn2/findpsw/findpsw_index");
        }
        Text {
            font: constant.labelFont;
            color: constant.colorLight;
            text: "<a href=\"link\">%1</a>".arg(qsTr("Register account"));
            //onLinkActivated: signalCenter.openBrowser("http://zc.qq.com/chs/index.html");
        }
    }


/*

    function login(vcode, vcodeMd5){
        unField.text = unField.text.replace(/\s/g, "");
        pwField.text = pwField.text.replace(/\s/g, "");
        var un = phoneNumberCheck.checked ? pnField.text : unField.text;
        if (un == "" || pwField.text == "") return;
        loading = true;
        var opt = {
            isphone: phoneNumberCheck.checked,
            un: un,
            passwd: pwField.text
        }
        if (vcode){
            opt.vcode = vcode;
            opt.vcode_md5 = vcodeMd5;
        }
        function s(){
            loading = false;
            signalCenter.showMessage(qsTr("Login success!"));
            pageStack.pop();
        }
        function f(err, obj){
            loading = false;
            signalCenter.showMessage(err);
            if (obj && obj.anti && obj.anti.need_vcode === "1"){
                signalCenter.needVCode(page, obj.anti.vcode_md5, obj.anti.vcode_pic_url);
                return;
            }
            pwField.forceActiveFocus();
        }
        Script.login(opt, s, f);
    }

    Connections {
        target: signalCenter;
        onVcodeSent: {
            if (caller === page){
                login(vcode, vcodeMd5);
            }
        }
    }

    ViewHeader {
        id: viewHeader;
        title: page.title;
    }

    Flickable {
        id: view;
        anchors { fill: parent; topMargin: viewHeader.height; }
        contentWidth: parent.width;
        contentHeight: contentCol.height + constant.paddingLarge

        Column {
            id: contentCol;
            anchors {
                left: parent.left; right: parent.right;
                top: parent.top; margins: constant.paddingLarge;
            }
            spacing: constant.paddingLarge;
            Text {
                text: phoneNumberCheck.checked ? qsTr("Phone number")
                                               : qsTr("ID or e-mail");
                font: constant.subTitleFont;
                color: constant.colorMid;
            }
            Flipable {
                id: flipable;
                width: parent.width;
                height: privateStyle.textFieldHeight;
                front: TextField {
                    id: unField;
                    enabled: !loading;
                    width: parent.width;
                    placeholderText: qsTr("Tap to input");
                    //platformInverted: tbsettings.whiteTheme;
                    inputMethodHints: Qt.ImhNoAutoUppercase;
                    KeyNavigation.down: pwField;
                    Keys.onPressed: {
                        if (event.key == Qt.Key_Select
                                ||event.key == Qt.Key_Enter
                                ||event.key == Qt.Key_Return){
                            pwField.forceActiveFocus();
                            event.accepted = true;
                        }
                    }
                }
                back: TextField {
                    id: pnField;
                    enabled: !loading;
                    width: parent.width;
                    placeholderText: qsTr("Tap to input");
                    //platformInverted: tbsettings.whiteTheme;
                    inputMethodHints: Qt.ImhDialableCharactersOnly;
                    KeyNavigation.down: pwField;
                    Keys.onPressed: {
                        if (event.key == Qt.Key_Select
                                ||event.key == Qt.Key_Enter
                                ||event.key == Qt.Key_Return){
                            pwField.forceActiveFocus();
                            event.accepted = true;
                        }
                    }
                }
                transform: Rotation {
                    id: flipRot;
                    origin: Qt.vector3d(flipable.width/2, flipable.height/2, 0);
                    axis: Qt.vector3d(0, 1, 0);
                    angle: 0;
                }
                states: State {
                    name: "back";
                    PropertyChanges {
                        target: flipRot; angle: 180;
                    }
                    when: phoneNumberCheck.checked;
                }
                transitions: Transition {
                    RotationAnimation {
                        direction: RotationAnimation.Clockwise;
                    }
                }
            }
            Text {
                text: qsTr("Password");
                font: constant.subTitleFont;
                color: constant.colorMid;
            }
            TextField {
                id: pwField;
                width: parent.width;
                enabled: !loading;
                placeholderText: qsTr("Tap to input");
                //platformInverted: tbsettings.whiteTheme;
                echoMode: TextInput.Password;
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText;
                KeyNavigation.up: flipable.state === "" ? unField : pnField;
                KeyNavigation.down: phoneNumberCheck;
                Keys.onPressed: {
                    if (event.key == Qt.Key_Select
                            ||event.key == Qt.Key_Enter
                            ||event.key == Qt.Key_Return){
                        loginBtn.clicked();
                        event.accepted = true;
                    }
                }
            }
            Text {
                font: constant.labelFont;
                color: constant.colorLight;
                text: "<a href=\"link\">%1</a>".arg(qsTr("Forget password?"));
                onLinkActivated: signalCenter.openBrowser("https://passport.baidu.com/?getpass_index");
            }
            Button {
                id: loginBtn;
                enabled: !loading
                         && (phoneNumberCheck.checked?pnField.text:unField.text)!==""
                         && pwField.text != "";
                anchors {
                    left: parent.left; right: parent.right;
                    margins: constant.paddingLarge*3;
                }
                text: qsTr("Login");
                //platformInverted: tbsettings.whiteTheme;
                onClicked: login();
            }
            Item { width: 1; height: 1; }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter;
                text: qsTr("Don't have a Baidu account?");
                font: constant.titleFont;
                color: constant.colorLight;
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter;
                text: "<a href=\"link\">%1</a>".arg(qsTr("Click to register"));
                font: constant.labelFont;
                color: constant.colorLight;
                onLinkActivated: signalCenter.openBrowser("http://wappass.baidu.com/passport/reg");
            }
        }
    }
*/
}
