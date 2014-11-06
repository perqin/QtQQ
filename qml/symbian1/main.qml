import QtQuick 1.0
import QQItemInfo 1.0
import com.nokia.symbian 1.0

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
                /*此处需要去获取验证码*/
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
        onTextChanged: {
            myqq.userQQ = text
        }
    }
    TextField{
        id: fileld_password
        anchors.horizontalCenter: field_account.horizontalCenter
        anchors.top: field_account.bottom
        anchors.topMargin: 10
        implicitWidth: field_account.implicitWidth
        onTextChanged: {
            myqq.userPassword = text
        }
    }

    Button{
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: fileld_password.bottom
        anchors.topMargin: 20
        text: "    登  录    "
        onClicked: {
            if(myqq.userQQ==""||myqq.userPassword=="")
                return

            myqq.loginStatus = QQ.Logining//登录状态设置为登录中
            myqq.checkAccount(testQQFinished)//先检测qq号是否需要验证码
        }
    }
}
