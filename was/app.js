const express = require('express')
const app = express()
const router = express.Router()
const port = 8899
const mysql = require('mysql');  // mysql 모듈 로드
const conn = {  // mysql 접속 설정
    host: '127.0.0.1',
    port: '3306',
    user: 'lunit',
    password: 'lunit-test',
    database: 'lunit'
};

const fileUpload = require('express-fileupload'); 
const cors = require('cors'); 
const fs = require('fs'); 
const mime = require('mime');

const dataLocation = "./lunit_data/";
const saliencyMapLocation = "./saliency_map/";
const downloadLocation = "http://localhost:8080/files/images/";

app.use(cors()); 
app.use(fileUpload());

var connection = mysql.createConnection(conn);
connection.connect();

var statusDesc = new Array();
var queryString = "select * from lunit.imageStatus";

connection.query(queryString, function (err, results, fields) {
  var jsonArray = new Array();

  for (var i = 0; i < results.length; i++) {
    statusDesc.push(results[i].statusDesc);
  }
});

function errorResponse(res, errorCode, errorMessage){
  var jsonObj = new Object();
  var jsonArray = new Array();

  jsonObj.error = errorMessage;
  jsonObj = JSON.stringify(jsonObj);
  jsonArray.push(JSON.parse(jsonObj));
  res.status(errorCode).json(jsonArray);
  res.end();
}

app.get('/images', (req, res) => {
  var queryString = "select * from lunit.imageMeta, lunit.imageStatus where lunit.imageMeta.status = lunit.imageStatus.statusId";

  console.log(queryString);
  connection.query(queryString, res, function (err, results, fields) {
    var jsonArray = new Array();
  
    if (err) {
        console.log(err);
        return;
    }
    
    if( 0 == results.length){
      errorResponse(res, 200, "images not found");
      return;
  }

    for (var i = 0; i < results.length; i++) {
      jsonArray.push(getImageMetaResult(results[i]));
    }
    res.status(200).json(jsonArray);
    res.end();
  });
})

app.get('/files/images/:imageFile', (req, res) => {
  var filePath = dataLocation;
  var fileName = req.params.imageFile;

  if( fs.existsSync(filePath+fileName)){
    var mimeType = mime.getType(filePath+fileName);

    res.setHeader('Content-disposition', 'attachment; filename=' + fileName);
    res.setHeader('Content-type', mimeType);

    res.download(filePath+fileName);
  }
})

function buildImageMetaAndInsert(newFileName, originalName){
  var queryString = "insert into `lunit`.`imageMeta` (`imageId`, `desc`, `uploadedTime`, `status`, `uploadedLocation`, `maxXIndex`, `maxYIndex`) VALUES (";
  var timestampString = getFormatDate(new Date());
  queryString += "'" + newFileName + "', ";
  queryString += "'" + originalName + "', ";;
  queryString += "'" + timestampString  + "', '0', ";;
  queryString += "'" + downloadLocation + newFileName + ".jpg', "
  queryString += " '-1', '-1');";

  console.log(queryString);
  
  connection.query(queryString, function (err, results, fields) {
    if (err) {
        console.log(err);
        return -1;
    }
  });

  return timestampString;
}

function startPatchfying(imageId, newFileName){

}

app.post('/images', (req, res) => {
  var newImageId = generateUUID();
  var newFilePath = dataLocation + newImageId + ".jpg";
  let uploadFile = req.files.image;
  const fileName = req.files.image.name; 
  uploadFile.mv(newFilePath, function (err) { 
      if (err) { 
        return res.status(500).send(err); 
      } 
    } 
  ); 

  var jsonArray = new Array();
  var jsonObj = new Object();
  
  var timeStamp = buildImageMetaAndInsert(newImageId, fileName);

  if( -1 == timeStamp ){
    errorResponse(res, 500, "MySQL Insert Error");
    return;
  }

  jsonObj.imageId = newImageId;
  jsonObj.uploaded_at = timeStamp;
  jsonObj.link = downloadLocation + newImageId + ".jpg";
  jsonObj.num_x_indices = "null";
  jsonObj.num_y_indices = "null";
  jsonObj.status = "uploaded";
  jsonObj = JSON.stringify(jsonObj);
  jsonArray.push(JSON.parse(jsonObj));
  res.status(201).json(jsonArray);
  
  startPatchfying(newImageId);
})

function getImageMetaResult(result){
  var jsonObj = new Object();

  jsonObj.imageId = result.imageId;
  jsonObj.uploaded_at = result.uploadedTime;
  jsonObj.link = result.uploadedLocation;
  jsonObj.num_x_indices = ( result.maxXIndex == -1) ? "null" : result.maxXIndex;
  jsonObj.num_y_indices = ( result.maxYIndex == -1) ? "null" : result.maxYIndex;
  jsonObj.status = result.statusDesc;
  jsonObj = JSON.stringify(jsonObj);
  console.log(jsonObj);
  return JSON.parse(jsonObj);
}

app.get('/images/:imageId', (req, res) => {

  var queryString = "select * from lunit.imageMeta, lunit.imageStatus where lunit.imageMeta.status = lunit.imageStatus.statusId and imageId='" + req.params.imageId + "' limit 1";

  console.log(queryString);
  connection.query(queryString, res, function (err, results, fields) { // testQuery 실행
    var jsonArray = new Array();
  
    if (err) {
        console.log(err);
        return;
    }

    if( 0 == results.length){
        errorResponse(res, 404, "not found");
        return;
    }

    jsonArray.push(getImageMetaResult(results[0]));
    res.status(200).json(jsonArray);
  });
})

app.get('/images/:imageId/:x/:y', (req, res) => {
  var queryString = "select * from lunit.imageMeta, lunit.imageStatus where lunit.imageMeta.status = lunit.imageStatus.statusId and imageId='" + req.params.imageId + "' limit 1";

  connection.query(queryString, res, function (err, results, fields) { // testQuery 실행
    var jsonArray = new Array();
  
    if (err) {
        console.log(err);
        errorResponse(res, 500, "MySQL select Error");
        return;
    }

    if( 0 == results[0].statusId || 1 == results[0].statusId){
      errorResponse(res, 400, "this image is not patchified yet");
      return;
    }
  
    if( req.params.x > results[0].maxXIndex || req.params.y > results[0].maxYIndex){
      errorResponse(res, 404, "not found");
      return;
    }

    queryString = "select * from lunit.patchImageMeta where imageId='" + req.params.imageId + "' and xIndex='" + req.params.x +"' and yIndex='" + req.params.y + "' limit 1;";
    connection.query(queryString, res, function (err, patchResults, fields) { // testQuery 실행
      var jsonArray = new Array();
      console.log(patchResults[0]);
      if (err) {
          console.log(err);
          errorResponse(res, 500, "MySQL select Error");
          return;
      }
  
        var jsonArray = new Array();
        var jsonObj = new Object();
        
        jsonObj.imageId = patchResults[0].imageId;
        jsonObj.x_index = patchResults[0].xIndex;
        jsonObj.y_index = patchResults[0].yIndex;
        jsonObj.link = patchResults[0].patchImageLocation;
        jsonObj.saliency_status = statusDesc[patchResults[0].saliencyStatus];
        jsonObj.histogram_status = statusDesc[patchResults[0].histogramStatus];
        jsonObj = JSON.stringify(jsonObj);
        jsonArray.push(JSON.parse(jsonObj));
        res.status(200).json(jsonArray);
    });
  });
})


app.get('/images/:imageId/:x/:y/saliencyMap', (req, res) => {
    
  var queryString = "select * from lunit.patchImageMeta, lunit.imageStatus where lunit.patchImageMeta.histogramStatus = lunit.imageStatus.statusId and imageId='" + req.params.imageId + "' and xIndex='" + req.params.x + "' and yIndex='" + req.params.y + "' limit 1";

  connection.query(queryString, res, function (err, results, fields) { // testQuery 실행
    if (err) {
      console.log(err);
      errorResponse(res, 500, "MySQL select Error");
      return;
    }
    console.log(results[0]);

    if( 0 == results.length){
      errorResponse(res, 404, "not found");
      return;
    }

    if( 3 == results[0].saliencyStatus )
    {
      errorResponse(res, 400, "the saliency map of this patch image is not calculated yet");
      return;
    }
    var filePath = saliencyMapLocation;
    var fileName = req.params.imageId + "_" + req.params.x + "_" + req.params.y + ".jpg";

    console.log(fileName);
    if( fs.existsSync(filePath+fileName)){
      var mimeType = mime.getType(filePath+fileName);

      res.setHeader('Content-disposition', 'attachment; filename=' + fileName);
      res.setHeader('Content-type', mimeType);

      res.download(filePath+fileName);
    }
  });
})

app.get('/images/:imageId/:x/:y/histogram', (req, res) => {
  var queryString = "select * from lunit.patchImageMeta, lunit.imageStatus where lunit.patchImageMeta.histogramStatus = lunit.imageStatus.statusId and imageId='" + req.params.imageId + "' and xIndex='" + req.params.x + "' and yIndex='" + req.params.y + "' limit 1";

  connection.query(queryString, res, function (err, results, fields) { // testQuery 실행
    if (err) {
      console.log(err);
      errorResponse(res, 500, "MySQL select Error");
      return;
    }
    console.log(results[0]);

    if( 0 == results.length){
      errorResponse(res, 404, "not found");
      return;
    }

    if( 3 == results[0].histogramStatus )
    {
      errorResponse(res, 400, "the histogram of this patch image is not calculated yet");
      return;
    }

    var jsonArray = new Array();
    var jsonObj = new Object();
    
    jsonObj.data = results[0].histogram;
    jsonObj = JSON.stringify(jsonObj);
    jsonArray.push(JSON.parse(jsonObj));
    res.status(200).json(jsonArray);

  });
})

app.listen(port, () => {
  console.log(`Example app listening on port ${port}`);
})

function generateUUID() { // Public Domain/MIT
  var d = new Date().getTime();//Timestamp
  var d2 = ((typeof performance !== 'undefined') && performance.now && (performance.now()*1000)) || 0;//Time in microseconds since page-load or 0 if unsupported
  return 'xxxxxxxx'.replace(/[xy]/g, function(c) {
      var r = Math.random() * 16;//random number between 0 and 16
      if(d > 0){//Use timestamp until depleted
          r = (d + r)%16 | 0;
          d = Math.floor(d/16);
      } else {//Use microseconds since page-load if supported
          r = (d2 + r)%16 | 0;
          d2 = Math.floor(d2/16);
      }
      return (c === 'x' ? r : (r & 0x3 | 0x8)).toString(16);
  });
}

function getFormatDate(date){
  var year = date.getFullYear();
  var month = (1 + date.getMonth());
  month = month > 10 ? month : '0' + month; // 10이 넘지 않으면 앞에 0을 붙인다
  var day = date.getDate();
  day = day > 10 ? day : '0' + day; // 10이 넘지 않으면 앞에 0을 붙인다
  var hours = date.getHours();
  hours = hours > 10 ? hours : '0' + hours; // 10이 넘지 않으면 앞에 0을 붙인다
  var minutes = date.getMinutes();
  minutes =  minutes > 10 ? minutes : '0' + minutes; // 10이 넘지 않으면 앞에 0을 붙인다
  var seconds = date.getSeconds();
  seconds = seconds > 10 ? seconds : '0' + seconds; // 10이 넘지 않으면 앞에 0을 붙인다

  // return year + '-' + month + '-' + day;
  return `${year}-${month}-${day} ${hours}:${minutes}:${seconds} `
}