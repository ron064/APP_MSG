var initialised = false;
var last_msg = 0;
var current_msg = 0;
var Total_lost = 0;
Pebble.addEventListener("ready", function() {
    initialised = true;
});
Pebble.addEventListener('appmessage', function(e) {
	current_msg=e.payload.Count;
	console.log("Appmessage received:"+current_msg);
	if (current_msg == 1)
		console.log("Appmessage received: "+JSON.stringify(e.payload)); // show data just once
	var orgdata = e.payload.Data;
	var results = [];
	for(var i = 0; i < orgdata.length; i += 2) {
		var combined = ((orgdata[i+1] & 0xFF) << 8) | (orgdata[i] & 0xFF);
		if(combined >= 0x8000) 
			combined -= 0x10000;
		results.push(combined);
	}
	console.log("count: "+current_msg+" original: "+ e.payload.XYZ+ " Recived Data: "+ results[0]+" "+results[1]+" "+results[2]);
	var lost_msg = current_msg -1 - last_msg;
	Total_lost = Total_lost + lost_msg;
	if (lost_msg > 0)
		console.log("lost(now/total): "+ lost_msg+"/"+Total_lost+" recived/Previus: "+current_msg+"/"+ last_msg );
	last_msg = current_msg;
});