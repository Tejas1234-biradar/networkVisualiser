import QtQuick 2.15
import QtGraphs



Rectangle {
    width: 640
    height: 480
    property var points: []

    GraphsView {
        anchors.fill: parent

        LineSeries {
            id: lineSeries
            color: "dodgerblue"
            width: 2
            axisX: ValueAxis {
                min: 0
                max: 10
            }
            axisY: ValueAxis {
                min: 0
                max: 10
            }
            points: graphModel.points
        }
    }
}
