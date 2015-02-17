var express = require('express');
var router = express.Router();
var fs = require("fs");

var key = "password1"; //worst Password EVER

/*
fs.writeFile( "./dish.json", JSON.stringify( {lat:0,lon:0} 
), "utf8", function(err) {
  console.log(err);
  if(err) throw err;
  console.log('saved');
});
fs.writeFile( "./user.json", JSON.stringify( {lat:0,lon:0} 
), "utf8", function(err) {
  console.log(err);
  if(err) throw err;
  console.log('saved');
});
*/

var dish_coords = JSON.parse(fs.readFileSync('./dish.json').toString());
var user_coords = JSON.parse(fs.readFileSync('./user.json').toString());
console.log(dish_coords);
console.log(user_coords);

/* GET error home page. */
router.get('/', function(req, res) {
  res.status(404).render('error', {message: 'Access not Authorized', error: {status:"No Key Specified",stack:"Please Specify your key."}});
});
/* GET error home page. */
router.get('/simple', function(req, res) {
  res.status(404).render('error', {message: 'Access not Authorized', error: {status:"No Key Specified",stack:"Please Specify your key."}});
});

/* GET home page. Map Interface */
router.get('/:key', function(req, res) {
  if(req.params.key == key) {
    res.render('map', { title: 'Track Pack - Long Range Wi-Fi', user: user_coords, dish: dish_coords});
  } else {
    res.status(404).render('error', {message: 'Access not Authorized', error: {status:"Incorrect Key Specified",stack:"Please Specify correct key."}});
  }
});
/* Get home page. Simple Interface */
router.get('/simple/:key', function(req, res) {
  if(req.params.key == key) {
    res.render('simple', { title: 'Track Pack - Long Range Wi-Fi', user: user_coords, dish: dish_coords});
  } else {
    res.status(404).render('error', {message: 'Access not Authorized', error: {status:"Incorrect Key Specified",stack:"Please Specify correct key."}});
  }
});

/* GET API Dish listing. */
router.get('/api/dish/:key/:lat/:lon', function(req, res) {
  if(req.params.key == key && (!isNaN(req.params.lat) && !isNaN(req.params.lon))) {
  	dish_coords.lat = Number(req.params.lat);
  	dish_coords.lon = Number(req.params.lon);
    fs.writeFileSync( "./dish.json", JSON.stringify( dish_coords ), "utf8", function(err) {
      console.log(err);
      if(err) throw err;
      console.log('saved');
    });
    console.log(fs.readFileSync('./dish.json').toString());
  	res.send(user_coords);
  } else {
  	res.send({});
  }
});

/* GET API User. */
router.get('/api/user/:key/:gps', function(req, res) {
  if(req.params.key == key && (!isNaN(req.params.gps.substring(0,req.params.gps.indexOf(','))) && !isNaN(req.params.gps.substring(req.params.gps.indexOf(',')+1)))) {
    //GPS in format lat,lon, parsed using substring and indexOf
  	user_coords.lat = Number(req.params.gps.substring(0,req.params.gps.indexOf(',')));
  	user_coords.lon = Number(req.params.gps.substring(req.params.gps.indexOf(',')+1));
    fs.writeFileSync( "./user.json", JSON.stringify( user_coords ), "utf8", function(err) {
      console.log(err);
      if(err) throw err;
      console.log('saved');
    });
    console.log(fs.readFileSync('./user.json').toString());
  	res.send(dish_coords);
  } else {
  	res.send({});
  }
});
module.exports = router;

/* GET API Dish listing. */
router.get('/api/dish/:key/', function(req, res) {
  if(req.params.key == key) {
    res.send(dish_coords);
  } else {
    res.send({});
  }
});

/* GET API User listing. */
router.get('/api/user/:key/', function(req, res) {
  if(req.params.key == key) {
    res.send(user_coords);
  } else {
    res.send({});
  }
});
