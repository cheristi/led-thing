var requestIp = require('request-ip');
var Datastore = require('nedb');
var request = require('request');
var bodyParser = require('body-parser'); // Middleware to read POST data
var express = require('express'),
  app = express(),
  port = Number(process.env.PORT || 8080);

app.listen(port, function() {
  console.log('Listening to port ' + port);
});

app.use(bodyParser.json());
// To parse form data:
app.use(bodyParser.urlencoded({
  extended: true
}));

var db = new Datastore({
  filename: 'nodes.db', // provide a path to the database file
  autoload: true, // automatically load the database
  timestampData: true // automatically add and manage the fields createdAt and updatedAt
});

app.get('/', function (req, res) {
  res.send('There is no spoon.');
});

app.get('/clearDb', function (req, res) {
  db.remove({}, { multi: true }, function (err, numRemoved) {
  res.sendStatus(200);
  });// Removing all documents with the 'match-all' query
});

// (Accessed at GET http://localhost:8080/goals)
app.get('/nodes', function(req, res) {
  db.find({}).sort({
    updatedAt: -1
  }).exec(function(err, goals) {
    if (err) res.send(err);
    res.json(goals);
  });
});

app.get('/register', function(req, res) {
  var node = {
    ip: requestIp.getClientIp(req)
  };
 // qry, update, but since both is the same ... use it for both.
  db.update(node, node, {upsert:true}, function(err, node) {
    if (err) res.send(err);
    console.log('new client registered');
    res.sendStatus(200);
  });
});

app.get('/unregister', function(req, res) {
  var req_ip = requestIp.getClientIp(req);
  db.remove({
    ip : req_ip
  }, {}, function(err, goal) {
    if (err) res.send(err);
    console.log('client unregistered');
    res.sendStatus(200);
  });
});

app.get('/style/:id', function(req, res) {
  db.find({}).sort({
    updatedAt: -1
  }).exec(function(err, nodes) {
    nodes.forEach(function(value){
      //strip ip in case of ipv6 https://stackoverflow.com/questions/31100703/stripping-ffff-prefix-from-request-connection-remoteaddress-nodejs
      var url = 'http://'+value.ip.replace(/^.*:/, '')+':8080/setStyle?'+'id='+req.params.id;

      request(url, function (error, response, body) {  });  //we even want to swallow errors, e.g. in case some node is currently not reachable.
    });

    if (err) res.send(err);
    res.sendStatus(200);
  });
});
app.get('/setStyle', function(req, res) {
   console.log("setStyle got the following : " + req.params);
   res.sendStatus(200);
});
