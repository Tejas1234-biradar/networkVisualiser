import QtQuick
import QtGraphs

GraphsView {
    id: root
    width: 400
    height: 300

    // Example LineSeries
    LineSeries {
        id: lineSeries
        // Sample static data
        XYPoint { x: 0; y: 0 }
        XYPoint { x: 1; y: 2 }
        XYPoint { x: 2; y: 3 }
        XYPoint { x: 3; y: 1 }
    }
}
