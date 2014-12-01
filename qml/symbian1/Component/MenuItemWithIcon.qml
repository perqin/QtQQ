import QtQuick 1.0
import com.nokia.symbian 1.0

MenuItem {
    id: root;

    property alias icon: iconImg.source;

    Image {
        id: iconImg;
        sourceSize.width: constant.graphicSizeMedium;
        sourceSize.height: constant.graphicSizeMedium;
        anchors.verticalCenter: parent.verticalCenter;
        anchors.left: parent.left;
        anchors.leftMargin: constant.paddingMedium;
    }
}
