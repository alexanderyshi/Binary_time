
// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
  }                     
);

Pebble.addEventListener('showConfiguration', function(e) {
  // Show config page
  Pebble.openURL('https://rawgit.com/alexanderyshi/Binary_time/master/config/index.html');
});

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = {};
  configData = JSON.parse(decodeURIComponent(e.response));
  console.log('Configuration page returned: ' + JSON.stringify(configData));
  
  // Send to watchapp
  Pebble.sendAppMessage(configData, function() {
    console.log('Send successful: ' + JSON.stringify(configData));
  }, function() {
    console.log('Send failed!');
  });
});
