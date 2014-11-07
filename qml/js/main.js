.pragma library

var signalCenter, settingsCenter;

function initialize(sic, sec) {
    signalCenter = sic;
    settingsCenter = sec;
    if (settingsCenter.autoLogin && settingsCenter.userQQ != "" && settingsCenter.userPass != "")
        login();
    else
        signalCenter.loginFail();
}

function login() {
    //
}
