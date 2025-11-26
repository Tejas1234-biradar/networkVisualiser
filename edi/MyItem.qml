import QtQuick
import DataService

Rectangle {

    id: root

    width: 100; height: 100
    color: "red"

    Text {
        id: counterText
        anchors.centerIn: parent // Centers the text horizontally and vertically within the parent Rectangle
        text: "Hello, QML!"
        color: "black"
        font.pointSize: 20
        font.bold: true

        DataService {
                onCounterChange: (countValue) => {
                                         console.log("in QML Rectangle DataService");
                                         counterText.text = countValue
                                 }
        }


    }

}
