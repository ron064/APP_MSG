var initialised = false;
var last_msg = 0;
var current_msg = 0;
var Total_lost = 0;
Pebble.addEventListener("ready", function() {
    initialised = true;
});
Pebble.addEventListener('appmessage', function(e) {
    //console.log("Appmessage received.");
	if (last_msg == 1)
		console.log("Appmessage received: "+JSON.stringify(e.payload));
	current_msg=e.payload.Count;
	var lost_msg = current_msg -1 - last_msg;
	Total_lost = Total_lost + lost_msg;
	if (lost_msg > 0)
		console.log("lost(now/total): "+ lost_msg+"/"+Total_lost+" recived/Previus: "+current_msg+"/"+ last_msg );
	last_msg = current_msg;
});