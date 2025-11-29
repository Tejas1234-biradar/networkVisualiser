import QtQuick 2.15
import QtGraphs
import DataService

Item {
    id:graphView
    width: 100
    property var dataPoint;

    // Rectangle{
    //     id: root
    //     width: 500
    //     height: 500
    //     color: "blue"

    //     Text {
    //         id: counterText
    //         anchors.centerIn: parent // Centers the text horizontally and vertically within the parent Rectangle
    //         text: "Hello, QML!"
    //         color: "black"
    //         font.pointSize: 20
    //         font.bold: true

    //         DataService {
    //                 onCounterChange: (countValue) => {
    //                      console.log("in QML Rectangle DataService");
    //                      counterText.text = countValue
    //              }
    //         }
    // }
// }
    GraphsView {
        anchors.fill: parent
        anchors.margins: 16
        theme: GraphsTheme {
            readonly property color c1: "#DBEB00"
            readonly property color c2: "#373F26"
            readonly property color c3: Qt.lighter(c2, 1.5)
            colorScheme: GraphsTheme.ColorScheme.Dark
            seriesColors: ["#2CDE85", "#DBEB00"]
            borderColors: ["#807040", "#706030"]
            grid.mainColor: c3
            grid.subColor: c2
            axisX.mainColor: c3
            axisY.mainColor: c3
            axisX.subColor: c2
            axisY.subColor: c2
            axisX.labelTextColor: c1
            axisY.labelTextColor: c1
        }
        axisX: ValueAxis {
            max: 5
            // tickInterval: 1
            // subTickCount: 9
            // labelDecimals: 1
        }
        axisY: ValueAxis {
            max: 10
            // tickInterval: 1
            // subTickCount: 4
            // labelDecimals: 1
        }

        component Marker : Rectangle {
            width: 16
            height: 16
            color: "#ffffff"
            radius: width * 0.5
            border.width: 4
            border.color: "#000000"
        }

        LineSeries {
            id: series
            name:"Data"
            width: 4
            //pointDelegate: Marker { }
            XYPoint { x: 0; y: 0 }
            XYPoint { x: 1; y: 2.1 }
            // XYPoint { x: 2; y: 3.3 }
            // XYPoint { x: 3; y: 2.1 }
            // XYPoint { x: 4; y: 4.9 }
            // XYPoint { x: 5; y: 3.0 }
                // DataService {
                //     onCounterChange: (countValue) => {
                //          console.log("in QML Rectangle DataService");
                //          graphView.dataPoint = {myPointProperty: Qt.point(countValue, countValue)};
                //          //graphView.dataPoint = {x: countValue, y: countValue};

                //     }

                // }
                Component.onCompleted: {
                    //console.log("graphView.dataPoints length:" + graphView.dataPoint);
                    console.info("In Graph Component Completed")
                       // var pt = graphView.dataPoint;
                       // append(pt.myPointProperty);
                }
        }

    }
}



