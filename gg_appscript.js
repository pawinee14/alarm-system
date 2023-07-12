//1hguuh4Zx72XVC1Zldm_vTtcUUKUA6iBUOoGnJUWLfqDWx5WlOJHqYkrt

var firebaseUrl = "XXXXXXXXX";

var CHANNEL_ACCESS_TOKEN = 'XXXXXXXXXXX'; 
var line_endpoint = 'https://api.line.me/v2/bot/message/reply';

function doPost(e) {
  var json = JSON.parse(e.postData.contents);  
  var reply_token= json.events[0].replyToken;
  if (typeof reply_token === 'undefined') {
    return;
  }
 
  var message = json.events[0].message.text;
  var reply_txt = message.toLowerCase().replace(" ", "");  

  if(reply_txt == "location" || reply_txt == "gps"){
    reply_txt = 1;
    var jsonText = { "value": {"location":  reply_txt  }};
    var base = FirebaseApp.getDatabaseByUrl(firebaseUrl);
    base.updateData("", jsonText);
    
  }else if(reply_txt == "open"){
    reply_txt = 1;
    var jsonText = { "value1": {"running":  reply_txt  }};
    var base = FirebaseApp.getDatabaseByUrl(firebaseUrl);
    base.updateData("", jsonText);
    
  }else if(reply_txt == "close"){
    reply_txt = 0;
    var jsonText = { "value1": {"running":  reply_txt  }};
    var base = FirebaseApp.getDatabaseByUrl(firebaseUrl);
    base.updateData("", jsonText);
  }
}
