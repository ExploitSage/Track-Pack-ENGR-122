google.maps.event.addDomListener(window, 'load', function() {
	var userCoords = new google.maps.LatLng(user.lat,user.lon);
	var dishCoords = new google.maps.LatLng(dish.lat,dish.lon);

	var mapOptions = {
		zoom: 15,
		center: userCoords,
		mapTypeId: google.maps.MapTypeId.HYBRID
	}
	var map = new google.maps.Map(document.getElementById("map-canvas"), mapOptions);

	// To add the marker to the map, use the 'map' property
	user_marker = new google.maps.Marker({
		position: userCoords,
		map: map,
		title:"User"
	});
	dish_marker = new google.maps.Marker({
		position: dishCoords,
		map: map,
		title:"Dish"
	});
});

//Update
window.setInterval(function() {
	$.getJSON('http://wifi.gustavemichel.com/api/dish/password1', function(json_data){
		dish_marker.setPosition(new google.maps.LatLng(json_data.lat,json_data.lon));
	});
	$.getJSON('http://wifi.gustavemichel.com/api/user/password1', function(json_data){
		user_marker.setPosition(new google.maps.LatLng(json_data.lat,json_data.lon));
	});
}, 2500);