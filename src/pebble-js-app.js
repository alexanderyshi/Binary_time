var myAPIKey = '5fad46ee28ed8a0ee37004dc14a80cbc';

function sendPebbleMessage(dictionary) {
  // Send to Pebble
  Pebble.sendAppMessage(dictionary,
    function(e) {
      console.log('Weather info sent to Pebble successfully!');
    },
    function(e) {
      console.log('Error sending weather info to Pebble!');
    }
  );    
}

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' +
  pos.coords.latitude + '&lon=' + pos.coords.longitude + '&appid=' + myAPIKey;
  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      // Temperature in Kelvin requires adjustment
      var temperature = Math.round(json.main.temp - 273.15);
      var dictionary = {
        'KEY_TEMPERATURE': temperature  
      };
      console.log('Temperature is ' + temperature);
      sendPebbleMessage(dictionary);
      // Conditions
      // var conditions = json.weather[0].main;      
      // console.log('Conditions are ' + conditions);
    }      
  );
}

function locationError(err) {
  console.log('Error requesting location!');
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');

    // Get the initial weather
    getWeather();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    getWeather();
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