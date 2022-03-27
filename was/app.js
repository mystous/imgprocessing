const express = require('express')
const app = express()
const port = 9180
const mysql = require('mysql');  // mysql 모듈 로드
const conn = {  // mysql 접속 설정
    host: '121.160.75.246',
    port: '3306',
    user: 'lunit',
    password: 'lunit-test',
    database: 'lunit'
};

var connection = mysql.createConnection(conn); // DB 커넥션 생성
connection.connect();   // DB 접속

app.get('/', (req, res) => {
  res.send('Hello World!');
})

app.get('/images', (req, res) => {
  res.send('Show all images');
})

app.post('/images', (req, res) => {
  //var uuid = generateUUID();
  var jsonArray = new Array();
  var jsonObj = new Object();

  jsonObj.imageId = generateUUID();
  jsonObj.uploaded_at = "2021-11-06T07:12:43";
  jsonObj.link = "http://localhost:8080/files/images/6115e7abac58.png";
  jsonObj.num_x_indices = "null";
  jsonObj.num_y_indices = "null";
  jsonObj.status = "uploaded";
  jsonObj = JSON.stringify(jsonObj);
  jsonArray.push(JSON.parse(jsonObj));
  res.status(201).json(jsonArray);
})

app.get('/images/:imageId', (req, res) => {
  var jsonArray = new Array();
  var jsonObj = new Object();

  jsonObj.imageId = req.params.imageId;
  jsonObj.uploaded_at = "2021-11-06T07:12:43";
  jsonObj.link = "http://localhost:8080/files/images/6115e7abac58.png";
  jsonObj.num_x_indices = "null";
  jsonObj.num_y_indices = "null";
  jsonObj.status = "uploaded";
  jsonObj = JSON.stringify(jsonObj);
  jsonArray.push(JSON.parse(jsonObj));

  res.status(201).json(jsonArray);
})

app.get('/images/:imageId/:x/:y', (req, res) => {
  res.send(req.params.imageId + " : " + req.params.x + " : " + req.params.y);
})

app.get('/images/:imageId/:x/:y/saliencyMap', (req, res) => {
  res.send("saliencyMap " + req.params.imageId + " : " + req.params.x + " : " + req.params.y);
})

app.get('/images/:imageId/:x/:y/histogram', (req, res) => {
  res.send("histogram "+ req.params.imageId + " : " + req.params.x + " : " + req.params.y);
})

app.listen(port, () => {
  console.log(`Example app listening on port ${port}`);
})



function generateUUID() { // Public Domain/MIT
  var d = new Date().getTime();//Timestamp
  var d2 = ((typeof performance !== 'undefined') && performance.now && (performance.now()*1000)) || 0;//Time in microseconds since page-load or 0 if unsupported
  return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
      var r = Math.random() * 16;//random number between 0 and 16
      if(d > 0){//Use timestamp until depleted
          r = (d + r)%16 | 0;
          d = Math.floor(d/16);
      } else {//Use microseconds since page-load if supported
          r = (d2 + r)%16 | 0;
          d2 = Math.floor(d2/16);
      }
      return (c === 'x' ? r : (r & 0x3 | 0x8)).toString(12);
  });
}