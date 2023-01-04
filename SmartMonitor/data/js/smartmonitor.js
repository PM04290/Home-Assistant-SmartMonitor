var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var client;
var jsonmqtt = JSON.parse("[]");
var timerMqtt = null;

var selectDataType = {
"0": "", "1": "Etat alarme", "2": "Etat volet", "3": "Etat lampe", "4": "Etat verrou", "49": "Etat binaire", "5": "Etat inter.", "6": "Etat Météo",
"7": "Puissance appar.", "8": "Qualité air", "9": "Pression Atmos.", "10": "Batterie", "11": "Dioxide carb.", "12": "Monoxide carb.",
"13": "Courant", "14": "Distance", "15": "Durée", "16": "Energie", "17": "Fréquence", "18": "Gas", "19": "Humidité", "20": "Luminosité",
"21": "Irradiance", "22": "Hygrométrie", "23": "Monétaire", "24": "Dioxyde d'azote", "25": "Monoxyde d'azote", "26": "Protoxyde d'azote",
"27": "Ozone", "28": "Concentr. part. <1µm", "29": "Concentr. part. <2.5µm", "30": "Concentr. part. <10µm",
"31": "Puissance", "32": "Fact. puissance", "33": "Précipitation", "34": "Précip. intensité", "35": "Pression", "36": "Puiss. réactive",
"37": "Force signal", "38": "Puiss. sonore", "39": "Vitesse", "40": "Dioxyde de Souffre", "41": "Température", "42": "C.O.V",
"43": "Voltage", "44": "Volume", "45": "Liquide", "46": "Poids", "47": "Vitesse vent", "48": "Elevation soleil",
};

function onLoad(event) {
	initWebSocket();
	$('#formconf').submit(function( event ) {
		event.preventDefault();
		var posting = $.post(event.currentTarget.action, $(this).serialize() );
		posting.done(function( data ) {
			$("#resultconf").fadeTo(100, 1);
			window.setTimeout(function() {$("#resultconf").fadeTo(500, 0)}, 2000);
			//console.log(data);
		});
	});
	$('#formgpio').submit(function( event ) {
		event.preventDefault();
		var posting = $.post(event.currentTarget.action, $(this).serialize() );
		posting.done(function( data ) {
			$("#resultgpio").fadeTo(100, 1);
			window.setTimeout(function() {$("#resultgpio").fadeTo(500, 0)}, 2000);
			//console.log(data);
		});
	});
	$('#formwifi').submit(function( event ) {
		event.preventDefault();
		var posting = $.post(event.currentTarget.action, $(this).serialize() );
		posting.done(function( data ) {
			$("#resultwifi").fadeTo(100, 1);
			window.setTimeout(function() {$("#resultwifi").fadeTo(500, 0)}, 2000);
			//console.log(data);
		});
	});
}

function initWebSocket() {
	//console.log('Trying to open a WebSocket connection...');
	websocket = new WebSocket(gateway);
	websocket.onopen    = WSonOpen;
	websocket.onclose   = WSonClose;
	websocket.onmessage = WSonMessage;
}

function WSonOpen(event) {
	//console.log('Connection opened');
}

function WSonClose(event) {
	//console.log('Connection closed');
}

function WSonMessage(event) {
	var cmd = "";
	//console.log(event.data);
	JSON.parse(event.data, (key, value) => {
		//console.log(cmd + ":" + key+'='+value);
		if (key === "cmd") {
			cmd = value;
		} else
		if (key != "" && typeof value !== 'object') {
			if (cmd === "html") {
				$(key).html( value );
			} else
			if (cmd === "replace") {
				$(key).replaceWith( value );
			}
		} else {
			if (cmd === "loadselect") {
				loadSelect();
			}
		}
	});
}

function loadSelect() {
	$("select[name^='pages_page']").empty();
	$("select[name^='pages_page']").append($('<option>', {value:"", text:""}));
	$("select[name^='pages_page']").append($('<option>', {value:"kp", text:"kp"}));

	$.each(selectDataType, function(key, value) {
		$("select[name^='pages_datatype']").each(function( i, e ) {
			$(this).append('<option value="' + key + '">' + value + '</option>');
		});
	});
	$("select[name^='pages_datatype']").each(function( i, e ) {
		var defval = $(this).attr('defval');
		$(this).val(defval);
	});
	$("input[name^='pages_id']").each(function( index, elt ) {
		if (index > 0) {
			$("select[name^='pages_page']").each(function( i, e ) {
				$(this).append($('<option>', {value:"p"+index, text:"p"+index}));
			});
		}
	});
	$("select[name^='pages_action']").change(function () {
		var arr = this.name.split("_");
		var optionSelected = $(this).find("option:selected");
		var valueSelected  = optionSelected.val();
		var namePage = "pages_page_"+arr[2]+"_"+arr[3];
		if (valueSelected == 2) {
			$("select[name='"+namePage+"']").removeAttr('disabled');
		}else{
			$("select[name='"+namePage+"']").attr('disabled', 'disabled');
		}
		var nameMQTT = "pages_mqtt_"+arr[2]+"_"+arr[3];
		if (valueSelected > 1) {
			$("input[name='"+nameMQTT+"']").removeAttr('disabled');
		}else{
			$("input[name='"+nameMQTT+"']").val('').trigger('input');
			$("input[name='"+nameMQTT+"']").attr('disabled', 'disabled');
		}
	});
	$("input[name^='pages_mqtt']").on('input', function() {
		var arr = this.name.split("_");
		var nameType = "pages_datatype_"+arr[2]+"_"+arr[3];
		$("select[name='"+nameType+"']").prop("disabled", this.value.length === 0);
	});
	$("select[name^='pages_page']").each(function( i, e ) {
		var page = $(this).attr('placeholder');
		$(this).val(page);
	});
	// GPIO
	$("select[name^='gpio_type']").change(function () {
		var arr = this.name.split("_");
		var optionSelected = $(this).find("option:selected");
		var valueSelected  = optionSelected.val();
		var coefGPIO = "gpio_coefa_"+arr[2];
		console.log(valueSelected);
		if (valueSelected == 3) {
			$("input[name='"+coefGPIO+"']").removeAttr('disabled');
		}else{
			$("input[name='"+coefGPIO+"']").attr('disabled', 'disabled');
		}
		coefGPIO = "gpio_coefb_"+arr[2];
		if (valueSelected == 3) {
			$("input[name='"+coefGPIO+"']").removeAttr('disabled');
		}else{
			$("input[name='"+coefGPIO+"']").attr('disabled', 'disabled');
		}
	});
	
	/*
	// Create a client instance https://github.com/eclipse/paho.mqtt.javascript
	client = new Paho.MQTT.Client($("input[name='MQTTSRV']").val(), 1884, "smarmon"+$("input[name='CODE']").val()+"_web");

	// set callback handlers
	client.onConnectionLost = onmqttConnectionLost;
	client.onMessageArrived = onmqttMessageArrived;

	// connect the client
	client.connect({onSuccess:onmqttConnect, userName:$("input[name='MQTTUSER']").val() , password:$("input[name='MQTTPASS']").val() });
	*/
}

function addpage(elt) {
	//console.log(elt);
	var nbpage = $("input[name^='pages_id']").length;
	$("#conf_page").append("<div id='newpage_"+nbpage+"'>page "+nbpage+"...</div>");
	websocket.send("addpage:"+nbpage);
}

/*
// called when the client connects
function onmqttConnect() {
  // Once a connection has been made, make a subscription and send a message.
  console.log("Mqtt onConnect");
  client.subscribe("aha/internal/#");
}

// called when the client loses its connection
function onmqttConnectionLost(responseObject) {
  if (responseObject.errorCode !== 0) {
    console.log("onConnectionLost:"+responseObject.errorMessage);
  }
}

Array.prototype.getIndexOf = function(el) {
  var arr = this;
  for (var i=0; i<arr.length; i++){
     console.log(arr[i].name);
     if(arr[i].name==el){
       return i;
     }
  }
  return -1;
}

function addPath(pathcomponents, arr, level) {
    let component = pathcomponents.shift();
    let comp = false;
    if (level ===0) {
        comp = arr.find(item => item.entity === component);
    } else if (level === 1) {
        comp = arr.find(item => item.device === component);
    }
    if (!comp) {
        if (level === 0) {
            comp =  {entity: component};
            arr.push(comp);
        } else if (level === 1) {
            comp =  {device: component};
            arr.push(comp);
        } else {
            var attr = component.split('=');
            arr[attr[0]] = attr[1];
        }
    }
    if(pathcomponents.length){
        if (level === 0) {
            addPath(pathcomponents, comp.devices || (comp.devices = []), level+1);
        } else if (level === 1) {
            addPath(pathcomponents, comp.attributes || (comp.attributes = {}), level+1);
        }
    }
    return arr;
}

// called when a message arrives
function onmqttMessageArrived(message) {
	var path = message.topic.replace("smartmonitor/","")+"="+message.payloadString.replaceAll('"','');
	//console.log(path);
	addPath(path.split('/'), jsonmqtt, 0);
	//console.log(jsonmqtt);
	//console.log(JSON.stringify(jsonmqtt));
	if (timerMqtt) { window.clearTimeout(timerMqtt); }
	timerMqtt = setTimeout(onMqttReady,2000);
}

function onMqttReady() {
	console.log("mqtt end");
	client.unsubscribe("aha/internal/#");
	for (const entry of Array.prototype.entries.call(jsonmqtt)) {
		//console.log(entry[1]);
		for (const [key, name] of Object.entries(entry[1])) {
			if (key === "entity") {
				console.log(name);
				$("select[name^='pages_entity']").each(function( i, e ) {
					$(this).append($('<option>', {value:name, text:name}));
				});
			}
		}
	}
}
*/