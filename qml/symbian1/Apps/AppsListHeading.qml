import QtQuick 1.0

Item {
    height: 26; width: screen.width;

    property alias title: titleLabel.text;

    Rectangle {
        color: "Grey";
        height: 2;
        anchors.left: parent.left; anchors.leftMargin: constant.paddingMedium;
        anchors.right: titleLabel.left; anchors.rightMargin: constant.paddingMedium;
        anchors.verticalCenter: parent.verticalCenter;
    }
    Text {
        id: titleLabel;
        color: "Grey";
        anchors.right: parent.right; anchors.rightMargin: constant.paddingMedium;
        anchors.verticalCenter: parent.verticalCenter;
    }
}
