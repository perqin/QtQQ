import QtQuick 1.0

Rectangle {
    id: root;

    property alias avatar: avatarImg.source;
    property alias stateIcon: stateIconImg.source;
    property alias nickname: nicknameLabel.text;
    property alias signature: signatureLabel.text;
    property alias paddingItem: padding;

    signal clicked;
    //signal avatarClicked;
    //signal stateClicked;
    //signal nicknameClicked;
    //signal signatureClicked;

    width: screen.width;
    height: constant.headerHeight;
    z: 10;
    opacity: 0.8;
    gradient: Gradient {
        GradientStop {
            color: "White";
            position: 0.0;
        }
        GradientStop {
            color: "#eeeeee";
            position: 1.0;
        }
    }

    Rectangle {
        id: mask;
        anchors.fill: parent;
        color: "Black";
        opacity: mouseArea.pressed ? 0.3 : 0;
    }
/*
    Image {
        anchors { left: parent.left; top: parent.top; }
        source: "../gfx/meegoTLCorner.png";
    }
    Image {
        anchors { right: parent.right; top: parent.top; }
        source: "../gfx/meegoTRCorner.png";
    }
*/
    Item {
        id: padding;
        anchors.fill: parent; anchors.margins: constant.paddingSmall;
        Image {
            id: avatarImg;
            source: "";
            anchors.left: parent.left;
            anchors.top: parent.top;
            height: parent.height; width: height;
        }
        Image {
            id: stateIconImg;
            source: "";
            anchors.left: avatarImg.right; anchors.leftMargin: constant.paddingSmall;
            anchors.top: parent.top; anchors.topMargin: constant.paddingSmall;
            height: 20; width: height;
        }
        Text {
            id: nicknameLabel;
            anchors.left: stateIconImg.right; anchors.leftMargin: constant.paddingSmall;
            anchors.right: parent.right; anchors.rightMargin: root.height;
            anchors.top: parent.top;
            font.pixelSize: constant.fontXLarge;
            color: "White";
            style: Text.Raised;
            styleColor: platformStyle.colorNormalMid;
            elide: Text.ElideRight;
        }
        Text {
            id: signatureLabel;
            anchors.left: avatarImg.right; anchors.leftMargin: constant.paddingSmall;
            anchors.right: parent.right; anchors.rightMargin: root.height;
            anchors.bottom: parent.bottom;
            font.pixelSize: constant.fontMedium;
            color: "LightGrey";
            style: Text.Raised;
            styleColor: platformStyle.colorNormalMid;
            elide: Text.ElideRight;
        }
    }

    MouseArea {
        id: mouseArea;
        anchors.fill: parent;
        onClicked: root.clicked();
    }
}
