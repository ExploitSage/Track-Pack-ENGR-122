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
		draggable:true,
		title:"User",
		icon:"http://maps.google.com/mapfiles/kml/paddle/blu-stars.png"
	});
	dish_marker = new google.maps.Marker({
		position: dishCoords,
		map: map,
		draggable:true,
		title:"Dish",
		icon:"http://maps.google.com/mapfiles/kml/paddle/D.png"
	});

	//Drag Adjust Events
	google.maps.event.addListener(user_marker, 'mouseup', function() {
		$.getJSON('/api/user/'+key+'/'+user_marker.position.A+','+user_marker.position.F, function(json_data){
			dish_marker.setPosition(new google.maps.LatLng(json_data.lat,json_data.lon));
		});
	});
	google.maps.event.addListener(dish_marker, 'mouseup', function() {
		$.getJSON('/api/dish/'+key+'/'+dish_marker.position.A+'/'+dish_marker.position.F, function(json_data){
			user_marker.setPosition(new google.maps.LatLng(json_data.lat,json_data.lon));
		});
	});
	//Update
	update = setInterval(function() {
		$.getJSON('/api/dish/'+key, function(json_data){
			dish_marker.setPosition(new google.maps.LatLng(json_data.lat,json_data.lon));
		});
		$.getJSON('/api/user/'+key, function(json_data){
			user_marker.setPosition(new google.maps.LatLng(json_data.lat,json_data.lon));
		});
	}, 2500);
});
