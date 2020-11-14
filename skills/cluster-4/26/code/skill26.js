/* Authors: Lesbeth Roque, Nnenna Eze
Skill 26 - Database (Team Skill)
Date: 11/3/2020
Team 15
*/

// Sources used:
// - http://www.tingodb.com/
// - https://github.com/sergeyksv/tingodb
// - http://whizzer.bu.edu/briefs/design-patterns/dp-db
// - https://stackoverflow.com/questions/34857458/reading-local-text-file-into-a-javascript-array

// Modules
var express = require('express');
var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http); //this didn't work for me so I put in its path
var Engine = require('tingodb')(); // added this in
var assert = require('assert');

var db = new Engine.Db('./', {});
var fs = require("fs");

// sych mode - https://stackoverflow.com/questions/34857458/reading-local-text-file-into-a-javascript-array
var text = fs.readFileSync("smoke.txt").toString('utf-8'); // convert text buffer to string we can read
var textByLine = text.split("\n")

// place in function and add data query, clear database before insert
// function collection(){
  var collection = db.collection("smoke_data"); // place into database
  for (i = 1; i < textByLine.length; i++){
    console.log(textByLine[i]);
    var elements = textByLine[i].split("\t"); // t is tab
    collection.insert([{time:parseFloat(elements[0]),ID:parseFloat(elements[1]),smoke:parseFloat(elements[2]),temp:parseFloat(elements[3])}],{w:1}, function(err, result) {
      assert.equal(null, err);
      collection.find().toArray(function(err,item){
        assert.equal(null, err);
        data = item;
        //db.close();
      });
    });
  };

// erase database
// collection.drop(function(err, res) {
//   if (err) throw err;
//   if (res) console.log("Collection deleted");
//   //db.close();
// });

// query to retrieve IDs with Smoke and Temp
var query = {};
collection.find(query).toArray(function(err, result){
  if (err) throw err;
  console.log(result);
});

// Points to index.html to serve webpage
app.get('/', function(req, res){
  res.sendFile(__dirname + '/index.html');
});

// When a new client connects
// var clientConnected = 0;
// this is just to ensure no new data is recorded during streaming
io.on('connection', function(socket){
  console.log('a user connected');
  io.emit('data_transmit', data);
  socket.on('disconnect', function(){
    console.log('user disconnected');
  });
});

// Listening on localhost:3000
http.listen(3000, function() {
  console.log('listening on *:3000');
});
