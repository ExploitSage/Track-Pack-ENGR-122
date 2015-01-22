var key = "password1";

var numeric_error = false;
var last_user;
var last_dish;
$(document).ready(function() {
	$( "#submit" ).addClass('btn-warning'); //loading

	//Set Values
	$.getJSON('/api/dish/'+key, function(json_data){
		last_dish = json_data;
		$( "#dish_lat" ).val(json_data.lat);
		$( "#dish_lon" ).val(json_data.lon);
	});
	$.getJSON('/api/user/'+key, function(json_data){
		last_user = json_data;
		$( "#user_lat" ).val(json_data.lat);
		$( "#user_lon" ).val(json_data.lon);
	});

	//update checker
	setInterval(function() {
		$.getJSON('/api/dish/'+key, function(json_data){
			if(json_data.lat != last_dish.lat || json_data.lon != last_dish.lon) {
				last_dish = json_data;
				$( "#dish_lat" ).val(json_data.lat);
				$( "#dish_lon" ).val(json_data.lon);
			}
		});
		$.getJSON('/api/user/'+key, function(json_data){
			if(json_data.lat != last_user.lat || json_data.lon != last_user.lon) {
				last_user = json_data;
				$( "#user_lat" ).val(json_data.lat);
				$( "#user_lon" ).val(json_data.lon);
			}
		});
	},2500);

	//Hook Submit
	$( "#submit" ).click(function() {
		if(!numeric_error) {
			$.getJSON('/api/user/'+key+'/'+$( "#user_lat" ).val()+','+$( "#user_lon" ).val(), function(json_data){
			
			});
			$.getJSON('/api/dish/'+key+'/'+$( "#dish_lat" ).val()+'/'+$( "#dish_lon" ).val(), function(json_data){
				
			});
			$( "#submit" ).removeClass('btn-primary').addClass('btn-success');
			//turn back blue after a time
			setTimeout( function() { $( "#submit" ).removeClass('btn-success').addClass('btn-primary'); }, 1000 );
		} else {
			$( "#submit" ).removeClass('btn-primary').addClass('btn-danger');
		}
	});

	//Hook TextBoxes (Error Checking)
	$('#user_lat').keyup(function() {
		numeric_check();
	});
	$('#user_lon').keyup(function() {
		numeric_check();
	});
	$('#dish_lat').keyup(function() {
		numeric_check();
	});
	$('#dish_lon').keyup(function() {
		numeric_check();
	});

	$( "#submit" ).removeClass('btn-warning').addClass('btn-primary'); //ready
});

//Check Values are Numeric, warn user if not
function numeric_check() {
	if(numeric_error) {
		$( "#submit" ).removeClass('btn-danger').addClass('btn-primary');
	}
	numeric_error = false;
	if( isNaN($('#user_lat').val()) ) {
		numeric_error = true;
	} else if( isNaN($('#user_lon').val()) ) {
		numeric_error = true;
	}else if( isNaN($('#dish_lat').val()) ) {
		numeric_error = true;
	}else if( isNaN($('#dish_lon').val()) ) {
		numeric_error = true;
	}

	if(numeric_error) {
		$( "#submit" ).removeClass('btn-primary').addClass('btn-danger')
	}
}