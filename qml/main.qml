import QtQuick 2.4
import QtMultimedia 5.10

Rectangle {
    id: root

    color: "black"

    property string file_source: ""

    property color cut_line_color: "red"
    property real cut_line_opacity: .3
    property real cut_line_width: 5

    property color cut_cortain_color: "black"
    property real cut_cortain_opacity: .9

    signal cut_up_changed(real value);
    signal cut_bottom_changed(real value);
    signal cut_left_changed(real value);
    signal cut_right_changed(real value);
    signal video_loaded(int w, int h);

    property bool __from_external: false
    property real __free_space_w: 0.0
    property real __free_space_h: 0.0

    property real oldW: 0.0
    property real oldH: 0.0

    function top_cut_change(value) {
        __from_external = true;

        var H = video.metaData.resolution.height;

        up_cut.y = value * (video.height - (__free_space_h * 2)) / H + __free_space_h;
    }

    function left_cut_change(value) {
        __from_external = true;

        var w = video.metaData.resolution.width;

        left_cut.x = value * (video.width - (__free_space_w * 2)) / w + __free_space_w;
    }

    function bottom_cut_change(value) {
        __from_external = true;

        var H = video.metaData.resolution.height;

        //down_cut.y = (value * (video.height - (__free_space_h * 2)) / H + __free_space_h) - cut_line_width;
        down_cut.y = ((value * (video.height - (__free_space_h * 2)))/H) - (cut_line_width) - __free_space_h;
    }

    function right_cut_change(value) {
        __from_external = true;

        var w = video.metaData.resolution.width;

        //right_cut.x = value * (video.width - (__free_space_w * 2)) / w + __free_space_w - right_cut.width;
        right_cut.x = ((value * (video.width - (__free_space_w * 2))) / w) - cut_line_width + __free_space_w;
    }

    function calculateRatios() {
        if (video.metaData.resolution === undefined) return;

        let ratio = video.metaData.resolution.width / video.metaData.resolution.height

        let direction = (root.height * ratio <= root.width); //TRUE -> Width : FALSE -> Height

        if (direction)
        {
            __free_space_w = (root.width - (root.height * ratio)) / 2.0
            __free_space_h = 0.0

            left_cut.x = __free_space_w + left_cut.width
            right_cut.x = root.width - right_cut.width - __free_space_w

            down_cut.y = root.height - down_cut.height
            up_cut.y = 0.0
        }
        else
        {
            __free_space_h = (root.height - (root.width / ratio)) / 2.0
            __free_space_w = 0.0

            up_cut.y = __free_space_h - up_cut.height
            down_cut.y = root.height - down_cut.height - __free_space_h

            right_cut.x = root.width - right_cut.width
            left_cut.x = 0.0
        }
    }

    Video {
        id: video
        anchors.fill: parent

        focus: true
        source: root.file_source
        loops: MediaPlayer.Infinite

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (video.playbackState == MediaPlayer.PlayingState) {
                    video.pause()
                } else {
                    video.play()
                }
            }
        }

        onErrorStringChanged: {
            console.log("ERROR ON VIDEO RENDER:", errorString);
        }

        onStatusChanged: {
            if (status == MediaPlayer.Loaded && seekable == true)
            {
                seek(20 * duration / 100);
                play();
                pause();

                calculateRatios();

                root.video_loaded(metaData.resolution.width, metaData.resolution.height)
            }
        }
    }

    Rectangle {
        id: up

        anchors.top: root.top
        anchors.bottom: undefined
        anchors.left: root.left
        anchors.right: root.right

        height: 0

        color: cut_cortain_color
        opacity: cut_cortain_opacity
    }

    Rectangle {
        id: down

        anchors.top: undefined
        anchors.bottom: root.bottom
        anchors.left: root.left
        anchors.right: root.right

        height: 0

        color: cut_cortain_color
        opacity: cut_cortain_opacity
    }

    Rectangle {
        id: left

        anchors.top: root.top
        anchors.bottom: root.bottom
        anchors.left: root.left
        anchors.right: undefined

        width: 0

        color: cut_cortain_color
        opacity: cut_cortain_opacity
    }

    Rectangle {
        id: right

        anchors.top: root.top
        anchors.bottom: root.bottom
        anchors.left: undefined
        anchors.right: root.right

        width: 0

        color: cut_cortain_color
        opacity: cut_cortain_opacity
    }

    Rectangle {
        id: up_cut

        color: cut_line_color
        opacity: cut_line_opacity

        anchors.left: root.left
        anchors.right: root.right

        height: cut_line_width
        y: up.height

        MouseArea {
            anchors.fill: parent

            drag.target: parent
            drag.axis: Drag.YAxis
            drag.smoothed: false
            drag.minimumY: __free_space_h
            drag.maximumY: root.height / 2 - 5
        }

        onYChanged: {
            up.height = y

            if (!__from_external && video.metaData.resolution !== undefined)
            {
                var H = video.metaData.resolution.height;
                root.cut_up_changed((H * (y - __free_space_h)) / (video.height - __free_space_h * 2));
            }

            __from_external = false
        }
    }

    Rectangle {
        id: down_cut

        color: cut_line_color
        opacity: cut_line_opacity

        anchors.left: root.left
        anchors.right: root.right

        height: cut_line_width
        y: root.height - height + 1

        MouseArea {
            anchors.fill: parent

            drag.target: parent
            drag.axis: Drag.YAxis
            drag.smoothed: false
            drag.minimumY: root.height / 2 + 5
            drag.maximumY: root.height - height - __free_space_h
        }

        onYChanged: {
            down.height = root.height - y

            if (!__from_external && video.metaData.resolution !== undefined)
            {                
                var H = video.metaData.resolution.height;
                root.cut_bottom_changed((H * (y + height + __free_space_h)) / (video.height - __free_space_h * 2));
            }

            __from_external = false
        }
    }

    Rectangle {
        id: left_cut

        color: cut_line_color
        opacity: cut_line_opacity

        anchors.top: root.top
        anchors.bottom: root.bottom

        width: cut_line_width
        x: 0

        MouseArea {
            anchors.fill: parent

            drag.target: parent
            drag.axis: Drag.XAxis
            drag.smoothed: false
            drag.minimumX: __free_space_w
            drag.maximumX: root.width / 2 - 5
        }

        onXChanged: {
            left.width = x

            if (!__from_external && video.metaData.resolution !== undefined)
            {
                var W = video.metaData.resolution.width
                root.cut_left_changed((W * (x - __free_space_w)) / (video.width - __free_space_w * 2));
            }

            __from_external = false


        }
    }

    Rectangle {
        id: right_cut

        color: cut_line_color
        opacity: cut_line_opacity

        anchors.top: root.top
        anchors.bottom: root.bottom

        width: cut_line_width
        x: root.width - width + 1

        MouseArea {
            anchors.fill: parent

            drag.target: parent
            drag.axis: Drag.XAxis
            drag.smoothed: false
            drag.minimumX: root.width / 2 + 5
            drag.maximumX: root.width - width - __free_space_w
        }

        onXChanged: {
            right.width = root.width - x

            if (!__from_external && video.metaData.resolution !== undefined)
            {
                var W = video.metaData.resolution.width;
                root.cut_right_changed((W * (x + width - __free_space_w)) / (video.width - __free_space_w * 2));
            }

            __from_external = false
        }
    }

    onFile_sourceChanged: {
        console.log("New video file", file_source);
    }

    onWidthChanged: {
        calculateRatios();

        if (video.metaData.resolution !== undefined)
        {
            root.video_loaded(video.metaData.resolution.width, video.metaData.resolution.height)
        }
    }

    onHeightChanged: {
        calculateRatios();

        if (video.metaData.resolution !== undefined)
        {
            root.video_loaded(video.metaData.resolution.width, video.metaData.resolution.height)
        }
    }
}
