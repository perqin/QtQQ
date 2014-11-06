import QtQuick 1.0
import QQItemInfo 1.0

QQ{
    id: root
    property variant loginReData//二次登陆后返回的数据(JSON格式)
    property variant userData//储存用户资料（JSON格式）
    property variant panelSize//存放主面板大小(网络数据)
    property string clientid//存放网络强求需要的clientid
    property variant friendListData//储存好友列表
    property string list_hash//获取好友列表时需要的hash
    property string ptwebqq//登录后返回的cookie
    property string psessionid: loginReData?loginReData.psessionid:""//登录后返回的数据
    property string vfwebqq: loginReData?loginReData.vfwebqq:""//登录后返回的数据
    

    Component.onCompleted: {
        clientid = getClientid()//设置clientid
    }

    onStateChanged: {
        editUserState()//改变在线状态
    }
    
    function random(min,max){
        return Math.floor(min+Math.random()*(max-min));
    }
    function getClientid() {
        return String(random(0, 99)) + String((new Date()).getTime() % 1000000)
    }
    
    function checkAccount(callbackFun){
        if(userQQ=="")
            return

        utility.socketAbort()//取消以前的网络请求
        var url2 = "https://ssl.ptlogin2.qq.com/check?uin="+myqq.userQQ+"&appid=1003903&r=0.08757076971232891"
        utility.httpGet(callbackFun, url2)

        utility.consoleLog("调用了检测qq号是否需要验证码")
    }

    function login(encryption_uin, code) {
        if( myqq.loginStatus == QQ.Logining ){
            var p = encryptionPassword(encryption_uin, code)
            var url1 = "https://ssl.ptlogin2.qq.com/login?u="+myqq.userQQ+"&p="+p+"&verifycode="+code+"&webqq_type=10&remember_uin=1&login2qq=1&aid=1003903&u1=http%3A%2F%2Fweb2.qq.com%2Floginproxy.html%3Flogin2qq%3D1%26webqq_type%3D10&h=1&ptredirect=0&ptlang=2052&daid=164&from_ui=1&pttype=1&dumy=&fp=loginerroralert&action=5-42-29419&mibao_css=m_webqq&t=1&g=1&js_type=0&js_ver=10087&login_sig=0RH3iE1ODTjmJJtKJ5MtDyoG*Q*pwgh2ABgmvw0E0zjdJpjPBbS*H9aZ4WRwLSFk&pt_uistyle=5"
            utility.httpGet(login1Finished, url1)
            utility.consoleLog("进行登录第一步")
        }
    }
    
    function login1Finished(error, data){//登录之后服务器返回的数据
        if(error){//如果出错了
            login(myqq.codeText)//再次请求
            return
        }

        if( myqq.loginStatus == QQ.Logining ){
            var list = data.split ("'");
            if( list[1]==0 ){
                var url = list[5]//先get一下返回数据中的url，来获取必要的Cookie
                utility.consoleLog("登录第一步完成")
                utility.httpGet(login2, url)//开始调用第二次登录
            }else{
                utility.consoleLog("登录出错，错误代码如下")
                console.debug(list[9])
                //myqq.showWarningInfo("登录失败："+list[9])
                myqq.error(list[9])
            }
        }
    }
    
    function login2() {
        if( myqq.loginStatus == QQ.Logining ){
            var url = "http://d.web2.qq.com/channel/login2"
            ptwebqq = utility.getCookie("ptwebqq")//储存cookie
            list_hash = getHash()//储存hash
            var data = 'r={"status":"'+myqq.stateToString+'","ptwebqq":"'+ptwebqq+'","passwd_sig":"","clientid":"'+clientid+'","psessionid":null}&clientid='+clientid+'&psessionid=null'
            data = encodeURI(data)
            utility.httpPost(login2Finished, url, data)
        }
    }
    function reLogin(){//用于掉线后重新登录
        var url = "http://d.web2.qq.com/channel/login2"
        ptwebqq = utility.getCookie("ptwebqq")//储存cookie
        var data = 'r={"status":"'+myqq.stateToString+'","ptwebqq":"'+ptwebqq+'","passwd_sig":"","clientid":"'+clientid+'","psessionid":null}&clientid='+clientid+'&psessionid=null'
        data = encodeURI(data)
        utility.httpPost(reLoginFinished, url, data, true)
    }
    function reLoginFinished(error, data) {
        if(error){
            reLogin()
            return
        }
        data = JSON.parse(data)
        if( data.retcode==0 ) {
            console.debug("重新登录完成")
            loginReData = data.result//将数据记录下来
            var poll2data = 'r={"clientid":"'+clientid+'","psessionid":"'+loginReData.psessionid+'","key":0,"ids":[]}&clientid='+clientid+'&psessionid='+loginReData.psessionid
            myqq.startPoll2(encodeURI(poll2data))//启动心跳包的post
        }else{
            console.debug("重新登录失败")
            showWarningInfo("QQ已掉线，请重新登录")
            root.loginStatus = QQ.WaitLogin//将登录状态设置为离线
        }
    }

    function login2Finished(error, data) {//二次登录，这次才是真正的登录
        if(error){//如果出错了
            login2(null)
            return
        }
        if( myqq.loginStatus == QQ.Logining ){
            var list = JSON.parse(data)
            if( list.retcode==0 ) {
                loginReData = list.result//将数据记录下来
                getUserData(myqq.userQQ, getDataFinished)//获取自己的资料
                myqq.openSqlDatabase();//登录完成后，打开数据库(用来储存聊天记录)
                myqq.loginStatus = QQ.LoginFinished//设置为登录成功
                var poll2data = 'r={"clientid":"'+clientid+'","psessionid":"'+loginReData.psessionid+'","key":0,"ids":[]}&clientid='+clientid+'&psessionid='+loginReData.psessionid
                myqq.startPoll2(encodeURI(poll2data))//启动心跳包的post
                var url = "http://q.qlogo.cn/headimg_dl?spec=240&dst_uin="+myqq.userQQ
                downloadImage(QQItemInfo.Friend, url, myqq.userQQ, "240", getAvatarFinished)//获取头像
            }else{
                //myqq.showWarningInfo("登陆出错，错误代码："+list.retcode)
            }
        }
    }
    
    function getUserData(uin, backFun) {//获取用户资料，登录完成后的操作
        var url = "http://s.web2.qq.com/api/get_friend_info2?tuin="+uin+"&verifysession=&code=&vfwebqq="+vfwebqq+"&t=1407324674215"
        utility.httpGet(backFun, url, true)//第三个参数为true，是使用高优先级的网络请求
    }
    
    function getDataFinished(error, data) {//获取用户资料成功后
        if(error){//如果出错了
            getUserData(myqq.userQQ, getDataFinished)//再次获取自己的资料
            return
        }
        var list = JSON.parse(data)
        if( list.retcode==0 ) {
            userData = list.result
            //console.debug("获取资料成功，我的昵称是："+userData.nick)
            root.nick = String(userData.nick)//储存昵称
            myqq.addLoginedQQInfo(userQQ, nick)//保存此账号的登录信息
        }else{
            getUserData(myqq.userQQ, getDataFinished)//再次获取自己的资料
        }
    }
    
    function getQQSignature(uin, backFun){//获取好友个性签名 backFun为签名获取成功后调用
        var url = "http://s.web2.qq.com/api/get_single_long_nick2?tuin="+uin+"&vfwebqq="+vfwebqq
        utility.httpGet(backFun, url)
    }
    function getFriendList(backFun) {//获取好友列表
        var url = "http://s.web2.qq.com/api/get_user_friends2"
        var data = 'r={"h":"hello","hash":"'+getHash()+'","vfwebqq":"'+vfwebqq+'"}'
        data = encodeURI(data)
        utility.httpPost(backFun, url, data, true)
    }
    
    function getGroupList(backFun) {//获取群列表
        var url = "http://s.web2.qq.com/api/get_group_name_list_mask2"
        var data = 'r={"hash":"'+getHash()+'","vfwebqq":"'+vfwebqq+'"}'
        data = encodeURI(data)
        utility.httpPost(backFun, url, data, true)
    }
    
    function getRecentList(backFun) {//获取最近联系人
        var url = "http://d.web2.qq.com/channel/get_recent_list2"
        var data = 'r={"vfwebqq":"'+vfwebqq+'","clientid":"'+clientid+'","psessionid":"'+loginReData.psessionid+'"}&clientid='+clientid+'&psessionid='+loginReData.psessionid
        data = encodeURI(data)
        utility.httpPost(backFun, url, data, true)
    }
    
    function getDiscusList(backFun) {//讨论组列表
        var url = "http://s.web2.qq.com/api/get_discus_list?clientid="+clientid+"&psessionid="+loginReData.psessionid+"&vfwebqq="+vfwebqq
        utility.httpGet(backFun, url, true)
    }
    
    function getFriendQQ( tuin, backFun ) {//获得好友真实的qq
        var url = "http://s.web2.qq.com/api/get_friend_uin2?tuin="+tuin+"&verifysession=&type=1&code=&vfwebqq="+vfwebqq
        utility.httpGet(backFun, url)
    }
    
    function getAvatarFinished( error, path, name ){//获得自己头像完成
        if(error){//如果出错
            downloadImage(QQItemInfo.Friend, url, myqq.userQQ, "240", getAvatarFinished)//重新获取头像
            return
        }

        myqq.avatar240 = path+"/"+name
    }
    
    function getFriendInfo( tuin,backFun ) {//获取好友资料
        var url = "http://s.web2.qq.com/api/get_friend_info2?tuin="+tuin+"&verifysession=&code=&vfwebqq="+vfwebqq
        utility.httpGet(backFun, url)
    }
    
    function editUserState(){
        if( loginStatus == QQ.LoginFinished ) {
            var url = "http://d.web2.qq.com/channel/change_status2?newstatus="+myqq.stateToString+"&clientid="+clientid+"&psessionid="+loginReData.psessionid
            utility.httpGet(editUserStateFinished, url)
        }
    }
    function editUserStateFinished(error, data){
        if(error){
            editUserState()//再次请求
            return
        }

        data = JSON.parse(data)
        if( data.retcode==0&&data.result=="ok" ){
            console.log("状态改变成功")
            if(root.state==QQ.Offline){//如果状态变为离线
                root.loginStatus = QQ.WaitLogin//改变登录状态
            }
        }
    }
    
    function sendFriendMessage(backFun, uin, message){
        while(message[message.length-1]=="\n"){
            message = message.substr(0, message.length-1)
        }

        var url = "http://d.web2.qq.com/channel/send_buddy_msg2"
        var data = 'r={"to":'+uin+',"face":549,"content":"[\\"'+message+'\\",\\"\\",[\\"font\\",{\\"name\\":\\"宋体\\",\\"size\\":\\"10\\",\\"style\\":[0,0,0],\\"color\\":\\"000000\\"}]]","msg_id":45070001,"clientid":"'+clientid+'","psessionid":"'+loginReData.psessionid+'"}&clientid='+clientid+'&psessionid='+loginReData.psessionid
        data = encodeURI(data)
        utility.httpPost(backFun, url, data, true)
    }
    
    function sendGroupMessage(backFun, uin, message){
        while(message[message.length-1]=="\n"){
            message = message.substr(0, message.length-1)
        }

        var url = "http://d.web2.qq.com/channel/send_qun_msg2"
        var data = 'r={"group_uin":'+uin+',"content":"[\\"'+message+'\\",\\"\\",[\\"font\\",{\\"name\\":\\"宋体\\",\\"size\\":\\"10\\",\\"style\\":[0,0,0],\\"color\\":\\"000000\\"}]]","msg_id":29780002,"clientid":"'+clientid+'","psessionid":"'+loginReData.psessionid+'"}&clientid='+clientid+'&psessionid='+loginReData.psessionid
        data = encodeURI(data)
        utility.httpPost(backFun, url, data, true)
    }
    
    function sendDiscuMessage(backFun, uin, message){
        while(message[message.length-1]=="\n"){
            message = message.substr(0, message.length-1)
        }
        
        var url = "http://d.web2.qq.com/channel/send_discu_msg2"
        var data = 'r={"did":'+uin+',"content":"[\\"'+message+'\\",\\"\\",[\\"font\\",{\\"name\\":\\"宋体\\",\\"size\\":\\"10\\",\\"style\\":[0,0,0],\\"color\\":\\"000000\\"}]]","msg_id":29780002,"clientid":"'+clientid+'","psessionid":"'+loginReData.psessionid+'"}&clientid='+clientid+'&psessionid='+loginReData.psessionid
        data = encodeURI(data)
        utility.httpPost(backFun, url, data, true)
    }
    function getGroupMembersList(callbackFun, gcode){//获取群成员列表
        var url = "http://s.web2.qq.com/api/get_group_info_ext2?gcode="+gcode+"&cb=undefined&vfwebqq="+vfwebqq
        utility.httpGet(callbackFun, url, true)
    }
    function getOnlineFriends(callbackFun){//获取在线好友列表
        var url = "http://d.web2.qq.com/channel/get_online_buddies2?clientid="+clientid+"&psessionid="+loginReData.psessionid
        utility.httpGet(callbackFun, url, true)
    }
}
