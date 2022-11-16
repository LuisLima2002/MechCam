const express = require('express');
const app = express();
const server = require('http').Server(app);
const url = require('url');
const fs = require("fs");
const WebSocket = require('ws');
const videoshow = require('videoshow')
const ffmpegPath = require('@ffmpeg-installer/ffmpeg').path;
const ffmpeg = require('fluent-ffmpeg');
ffmpeg.setFfmpegPath(ffmpegPath);

const port = process.env.PORT || 3000;

const express_config= require('./config/express.js');

express_config.init(app);

const wss1 = new WebSocket.Server({ noServer: true });
const wss2 = new WebSocket.Server({ noServer: true });
const framesToStore = 14.400
var frame = 0;
var canSave= true


function createVideo(){
  console.log("making video")
  let secondsToShowEachImage = 0.25
  let images = [ ]
  for (let i = 0; i <= framesToStore; i++) {
    images.push({path:"./images/frame"+i+".jpg",loop:secondsToShowEachImage})
  }
    let finalVideoPath = 'video.mp4'
    let videoOptions = {
      transition: false,
      videoBitrate: 512 ,
      videoCodec: 'libx264', 
      size: '90x90',
      outputOptions: ['-pix_fmt yuv420p'],
      format: 'mp4' 
    }
    videoshow(images, videoOptions)
    .save("video.mp4")
    .on('start', function (command) { 
      console.log('encoding ' + finalVideoPath + ' with command ' + command) 
    })
    .on('error', function (err, stdout, stderr) {
      console.log('error') 
      return Promise.reject(new Error(err)) 
    })
    .on('end', function (output) {
      // do stuff here when done
      frame=0
    })
}



wss1.on('connection', function connection(ws) {
  console.log("camera connected")
  ws.on('message', function incoming(message) {
    if(canSave && frame<=framesToStore){
      let filename = "./images/frame"+frame+".jpg";
        fs.writeFile(filename, message, "binary", (err) => {
          if (!err){ 
            console.log(`${filename} created successfully!`);
            frame++;}
            lastFrame= new Date()
            if(frame===framesToStore+1){
              createVideo();
            }
            canSave=false
            setTimeout(()=>{canSave=true},250)
        })
    }
    wss2.clients.forEach(function each(client) {
      if (client.readyState === WebSocket.OPEN) {
        client.send(message);
      }
    });
  });
});

//webbrowser websocket
wss2.on('connection', function connection(ws) {
  console.log("client connected")
  ws.on('message', function incoming(message) {
  // nothing here should be received
    wss1.clients.forEach(function each(client){
      if (client.readyState === WebSocket.OPEN) {
        console.log("Sending to espcam "+ message.toString())
        client.send(message.toString());
      }
    })
  });
});

server.on('upgrade', function upgrade(request, socket, head) {
  const pathname = url.parse(request.url).pathname;

  if (pathname === '/jpgstream_server') {
    wss1.handleUpgrade(request, socket, head, function done(ws) {
      wss1.emit('connection', ws, request);
    });
  } else if (pathname === '/jpgstream_client') {
    wss2.handleUpgrade(request, socket, head, function done(ws) {
      wss2.emit('connection', ws, request);
    });
  } else {
    socket.destroy();
  }
});

app.get('/', (req, res) => {
  	res.render('index', {});
});


server.listen(port, () => {
	  console.log(`App listening at http://localhost:${port}`)
})