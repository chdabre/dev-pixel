Vue.component('matrix', {
    template: '#matrix-template',
    props: ['editable', 'value', 'cclass', 'selected'],
    methods: {
        flipPixel: function (pixel, index) {
            if (this.editable !== undefined) {
                Vue.set(this.value, index, (pixel === 1) ? 0 : 1)
                this.$emit('input', this.value)
            }
        }
    }
})

var app = new Vue({
    el: '#app',
    data: {
        image: [],
        images: [],
        timelines: [{name: "Timeline 1", frames: [], speed: 100, repeat: true}],
        timeline: null,
        newTimeline: {name: "", frames: [], speed: 100, repeat: true},
        pixels: [],
        newPixel: {"ip":"192.168.1.0", "name": "Name", "intensity": 8},
        instantUpdate: false,
        lastPixel: null,
        imageIndex: null,
        imageSource: null,
        hex: "0000000000000000",
        importData: "",
        importTimeline: "",
        message: "",
        msgRepeat: false,
        previewIntervalId: null,
    },
    methods:{
        clearImage: function () {
            this.imageIndex = null
            this.image = new Array(64).fill(0)
        },
        addImage: function () {
            if(parseInt(this.image.join(''), 2) > 0) {
                this.images.push(this.image.slice())
            }
        },
        deleteImage: function() {
            if(this.imageIndex !== null) {
                if(this.imageSource == "saved"){
                    this.images.splice(this.imageIndex, 1)
                }else if (this.imageSource = "timeline") {
                    this.timeline.frames.splice(this.imageIndex, 1)
                }
                this.clearImage()
            }
        },
        getImage: function (index) {
            this.imageSource = "saved"
            this.imageIndex = index
            this.image = this.images[index]
        },
        sendImage: function (pixel) {
            this.lastPixel = pixel
            url = 'http://' + pixel.ip + '/?data=' + this.hex + '&intensity=' + pixel.intensity
            console.log("REQUEST: " + url)
           
            this.$http.get(url).then(response => {
                console.log("SUCCESS", response)
            }, response => {
                console.log("ERROR", response)
            })
        },
        sendMessage: function () {
            url = 'http://' + this.lastPixel.ip + '/?message=' + this.message
            if (this.msgRepeat) url += '&repeat=true'

            console.log("REQUEST: " + url)
           
            this.$http.get(url).then(response => {
                console.log("SUCCESS", response)
            }, response => {
                console.log("ERROR", response)
            })
        },
        deletePixel: function (pixel) {
            this.pixels.splice(this.pixels.indexOf(pixel), 1)
        },
        createPixel: function () {
            this.pixels.push(this.newPixel)
            this.newPixel = {"ip":"192.168.1.0", "name": "Name"}
        },
        importImages: function () {
            var importData = JSON.parse(this.importData).map(function (image) {
                return hextoBytes(image)
            })
            console.log(importData)
            this.images = this.images.concat(importData)
            this.importData = ""
        },
        importTimelines: function () {
            var importTimelines = JSON.parse(this.importTimeline)

            for(i in importTimelines){
                importTimelines[i].frames = importTimelines[i].frames.map(function (frame) {
                    return hextoBytes(frame)
                })
            }

            console.log(importTimelines)
            this.timelines = this.timelines.concat(importTimelines)
            this.importTimeline = ""
        },
        addToTimeline: function () {
            if(parseInt(this.image.join(''), 2) > 0) {
                this.timeline.frames.splice(this.imageIndex, 0, this.image.slice());
                this.imageIndex++
                this.getImageFromTimeline(this.imageIndex)
            }
        },
        getImageFromTimeline: function (index) {
            this.imageSource = "timeline"
            this.imageIndex = index
            this.image = this.timeline.frames[index]
        },
        playPreview: function () {
            this.previewIntervalId = setInterval(() => {
                this.imageIndex++

                if(this.imageIndex >= this.timeline.frames.length) {
                    if(this.timeline.repeat) {
                        this.imageIndex = 0
                    } else {
                        this.stopPreview()
                        return
                    }
                }

                this.getImageFromTimeline(this.imageIndex)

            }, this.timeline.speed)
        },
        stopPreview: function () {
            clearInterval(this.previewIntervalId)
            this.previewIntervalId = null
        },
        sendTimeline: function () {
            var hexFrames = this.timeline.frames.map(function (frame) {
                return bytesToHex(frame)
            })

            url = 'http://' + this.lastPixel.ip + '/?animation=' + hexFrames.join('') + '&speed=' + this.timeline.speed
            if (this.timeline.repeat) url += "&repeat=true"

            console.log("REQUEST: " + url)

            this.$http.get(url).then(response => {
                console.log("SUCCESS", response)
            }, response => {
                console.log("ERROR", response)
            })
        },
        getTimeLine: function (timeline) {
            this.timeline = timeline
            this.imageIndex = 0
        },
        createTimeline: function (){
            if(this.newTimeline.name.length > 0){
                this.timelines.push(JSON.parse(JSON.stringify(this.newTimeline)))
                this.imageIndex = 0
                this.timeline = this.timelines[this.timelines.length - 1]
                this.newTimeline = {name: "", frames: [], speed: 100, repeat: true}
            }
        },
        deleteTimeline: function () {
            this.timelines.splice(this.timelines.indexOf(this.timeline), 1)
            this.timeline = this.timelines[0]
        }
    },
    computed: {
        stringImages: function () {
            var imagesHex = this.images.map(function (image) {
                return bytesToHex(image)
            })

            return JSON.stringify(imagesHex)
        },
        stringTimelines: function () {
            var timelines = JSON.parse(JSON.stringify(this.timelines))
            console.log(timelines)
            for (i in timelines) {
              timelines[i].frames = timelines[i].frames.map(function (value) {
                    return bytesToHex(value)
                })
            }

            return JSON.stringify(timelines)
        }
    },
    watch: {
      image: function (){
        this.hex = bytesToHex(this.image)
        if(this.instantUpdate && this.lastPixel !== null) {
            this.sendImage(this.lastPixel)
        }
      },
      images: {
          handler: function () {
              localStorage.setItem('PixelImageStorage', JSON.stringify(this.images))
          },
          deep: true
      },
      timelines: {
          handler: function () {
              localStorage.setItem('PixelTimelineStorage', JSON.stringify(this.timelines))
          },
          deep: true
      },
      pixels: {
          handler: function () {
              if(this.lastPixel !== null) this.sendImage(this.lastPixel)
              localStorage.setItem('PixelStorage', JSON.stringify(this.pixels))
          },
          deep: true
      }
    },
    created: function(){
        this.image = new Array(64).fill(0)
        var localImages = JSON.parse(localStorage.getItem('PixelImageStorage'))  
        this.images = localImages !== null ? localImages : []

        var localTimelines = JSON.parse(localStorage.getItem('PixelTimelineStorage'))
        if (localTimelines){
             this.timelines = localTimelines
        }
        this.timeline = this.timelines[0]

        var localPixels = JSON.parse(localStorage.getItem('PixelStorage'))  
        this.pixels = localPixels !== null ? localPixels : [] 
        
        if(this.pixels.length > 0) {
            this.lastPixel = this.pixels[0]
        }
    }
})

function bytesToHex(bytes) {
    bytes = bytes.join('').match(/.{1,8}/g)
    var hex = ""
    for (byte in bytes) {
        hex += ("00" + parseInt(bytes[byte], 2).toString(16)).slice(-2)
    }

    return hex
}

function hextoBytes(hex) {
    var hexBytes = hex.match(/.{1,2}/g)
    var bytes = []
    for(byte in hexBytes) {
        var byteRow = ("00000000" + parseInt(hexBytes[byte], 16).toString(2)).slice(-8).split('')
        
        byteRow = byteRow.map(function (bit) {
            return parseInt(bit)
        })

        bytes = bytes.concat(byteRow)
    }

    return bytes
}
