// v0.3
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var client;
var jsonmqtt = JSON.parse("[]");
var timerMqtt = null;
var mqttstep = 0;
var sourcemqttrequest = null;
var sourcemodal = null;

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
	$('#myModal').on('show.bs.modal', function (event) {
		var button = $(event.relatedTarget) // Button that triggered the modal
		sourcemodal = button.data('source') // Extract info from data-* attributes
		console.log(sourcemodal);
	})
	$('#saveModal').click(function() {
		$("input[name='pages_mqtt_"+sourcemodal+"']").val($("#cnf-base").val()+$("#cnf-state").val()+$("#cnf-command").val()).trigger('input');
		$("input[name='pages_source_"+sourcemodal+"']").val($("#cnf-source").val());
		$("input[name='pages_icon_"+sourcemodal+"']").val($("#cnf-class").val());
		$("input[name='pages_unit_"+sourcemodal+"']").val($("#cnf-unit").val());
		$('#myModal').modal('hide');
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
		var nameSrc = "pages_source_"+arr[2]+"_"+arr[3];
		$("input[name='"+nameSrc+"']").prop("disabled", this.value.length === 0);
		var nameIcon = "pages_icon_"+arr[2]+"_"+arr[3];
		$("input[name='"+nameIcon+"']").prop("disabled", this.value.length === 0);
		var nameUnit = "pages_unit_"+arr[2]+"_"+arr[3];
		$("input[name='"+nameUnit+"']").prop("disabled", this.value.length === 0);
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
		if (valueSelected == 5) {
			$("input[name='"+coefGPIO+"']").removeAttr('disabled');
		}else{
			$("input[name='"+coefGPIO+"']").attr('disabled', 'disabled');
		}
		coefGPIO = "gpio_coefb_"+arr[2];
		if (valueSelected == 5) {
			$("input[name='"+coefGPIO+"']").removeAttr('disabled');
		}else{
			$("input[name='"+coefGPIO+"']").attr('disabled', 'disabled');
		}
	});
	
}

function addpage(elt) {
	var nbpage = $("input[name^='pages_id']").length;
	$("#conf_page").append("<tr id='newpage_"+nbpage+"'>page "+nbpage+"...</tr>");
	websocket.send("addpage:"+nbpage);
}

function initMqtt(elt) {
	sourcemqttrequest = elt;
	$(elt).prop("disabled",true);
	$("#mqttloading").html("Recherche en cours...");
	jsonmqtt = JSON.parse("[]");
	client = new Paho.MQTT.Client($("input[name='MQTTSRV']").val(), 1884, "smarmon"+$("input[name='CODE']").val()+"_web");
	client.onConnectionLost = onmqttConnectionLost;
	client.onMessageArrived = onmqttMessageArrived;
	client.connect({onSuccess:onmqttConnect, userName:$("input[name='MQTTUSER']").val() , password:$("input[name='MQTTPASS']").val() });
}

function onmqttConnect() {
  console.log("Mqtt onConnect");
  if (mqttstep === 0) {
	  client.subscribe("smartmonitor/binary_sensor/+/state");
	  client.subscribe("smartmonitor/binary_sensor/+/device_class");
	  client.subscribe("smartmonitor/sensor/+/state");
	  client.subscribe("smartmonitor/sensor/+/device_class");
	  client.subscribe("smartmonitor/sensor/+/unit_of_measurement");
	  client.subscribe("smartmonitor/switch/+/state");
	  client.subscribe("smartmonitor/light/+/state");
	  client.subscribe("smartmonitor/cover/+/state");
	  client.subscribe("smartmonitor/lock/+/state");
	  client.subscribe("smartmonitor/sun/#");
	  client.subscribe("smartmonitor/weather/#");
	  client.subscribe("smartmonitor/alarm_control_panel/#");
  }
  if (mqttstep === 1) {
	  client.subscribe("homeassistant/switch/#");
	  client.subscribe("homeassistant/light/#");
	  client.subscribe("homeassistant/cover/#");
	  client.subscribe("homeassistant/lock/#");
  }
  if (timerMqtt) { window.clearTimeout(timerMqtt); }
  timerMqtt = setTimeout(onMqttReady,2000);
}

function onmqttConnectionLost(responseObject) {
  if (responseObject.errorCode !== 0) {
    console.log("onConnectionLost:"+responseObject.errorMessage);
  }
}
	  
function addMessage(ismqtt, arrtopic, payload) {
	let elementType = arrtopic.shift(); // binary_sensor, sensor, switch, etc.
	if (ismqtt) {
		obj = JSON.parse(payload);
		let component = obj.name.replaceAll(" ","_").toLowerCase();
		let elt = jsonmqtt.find(item => item.elt === component);
		if (!elt) {
			console.log('mqtt element creation:'+component);
			console.log(obj);
			elt =  {elt: component, type:elementType, attributes: JSON.parse("{}")};
			jsonmqtt.push(elt);
		}
		if (elementType === "switch" || elementType === "light" || elementType === "cover" || elementType === "lock") {
			elt.base_topic = obj.state_topic;
			let stateTopic;
			if (obj.schema === "json") {
				stateTopic = ";/#|state";
			} else
			if (obj.json_attributes_topic !== "") {
				elt.base_topic = obj.json_attributes_topic;
				let stateAttrib = "state";
				if (obj.value_template) {
					const regex = /(?<=json.)[^.]+(?=\ )/
					stateAttrib = regex.exec(obj.value_template);
				}
				stateTopic = ";/#|"+stateAttrib;
			}
			elt.state_topic = stateTopic;
			//
			let cmd = obj.command_topic.substring(elt.base_topic.length+1);
			elt.command_topic = ";/"+cmd;
		}
	} else {
		let component = arrtopic.shift();
		let elt = jsonmqtt.find(item => item.elt === component);
		if (!elt) {
			elt =  {elt: component, type:elementType, attributes: JSON.parse("{}")};
			elt.base_topic = "smartmonitor/"+elementType+"/"+component;
			elt.state_topic = "/state";
			jsonmqtt.push(elt);
		}
		try {
			elt.attributes[arrtopic[0]] = JSON.parse(payload);
		} catch (error) {
			elt.attributes[arrtopic[0]] = payload;
		}
	}
}

function onmqttMessageArrived(message) {
	//console.log(message.topic);
	let ismqtt = message.topic.startsWith("homeassistant");
	let topic = message.topic.replace("smartmonitor/","").replace("homeassistant/","");
	let arrtopic = topic.split("/");
	addMessage(ismqtt, arrtopic, message.payloadString);
	if (timerMqtt) { window.clearTimeout(timerMqtt); }
	timerMqtt = setTimeout(onMqttReady,2000);
}

function onMqttReady() {
	client.disconnect();
	jsonmqtt.sort((a, b) => {
		const nameA = a.type+a.elt;
		const nameB = b.type+b.elt;
		if (nameA < nameB) {
			return -1;
		}
		if (nameA > nameB) {
			return 1;
		}
		return 0;
	});
	if (mqttstep === 0) {
		client.connect({onSuccess:onmqttConnect, userName:"toto" , password:"toto" });
		mqttstep = 1;
	} else
	if (mqttstep === 1) {
		jsonmqtt = jsonmqtt.filter(function(elt){ 
			return elt.hasOwnProperty('state_topic');
		});
		let oldtype = "";
		let target = $("#cnf-device");
		for (const entry of Array.prototype.entries.call(jsonmqtt)) {
			if (oldtype !== entry[1].type) {
				target = $("#cnf-device").append($('<optgroup>', {label:entry[1].type}));
				oldtype = entry[1].type;
			}
			target.append($('<option>', {value:entry[0], text:entry[1].elt}));
		}
		$(sourcemqttrequest).prop("disabled",false);
		console.log(jsonmqtt);
		$("#mqttloading").html("Mqtt chargé");
		window.setTimeout(function() {$("#mqttloading").html("")}, 2000);
		$("button[name^='btn_mqtt']").removeAttr('disabled');
	}
}

function onConfigDeviceChange(whosend) {
	let elt = $(whosend).children("option").filter(":selected").text();
	$("#cnf-entity").empty();
	$("#cnf-source").val("");
	$("#cnf-class").val();
	$("#cnf-unit").val("");
	$("#cnf-base").val("");
	$("#cnf-state").val("");
	$("#cnf-command").val("");
	$("#test-state").val("");
	for (const [devkey, devobj] of Array.prototype.entries.call(jsonmqtt)) {
		if (devobj.elt === elt) {
			$("#cnf-source").val(devobj.type);
			$("#cnf-class").val(devobj.attributes.device_class);
			$("#cnf-unit").val(devobj.attributes.unit_of_measurement);
			$("#cnf-base").val(devobj.base_topic);
			$("#cnf-state").val(devobj.state_topic);
			$("#cnf-command").val(devobj.command_topic);
			$("#test-state").val(devobj.attributes.state);
			//
			let excludeAttr = ['friendly_name', 'supported_features', 'device_class', 'unit_of_measurement'];
			for (let name in devobj.attributes) {
				if (typeof devobj.attributes[name] !== 'object' && !name.endsWith("_unit") && !excludeAttr.includes(name)) {
					$("#cnf-entity").append($('<option>', {value:name, text:name}));
				}
			}
			$("#cnf-entity").val('state').trigger('change');
		}
	}
}

function onConfigEntityChange(whosend) {
	let dev = $("#cnf-device").children("option").filter(":selected").val();
	let devobj = jsonmqtt[dev];
	let attrib = $(whosend).children("option").filter(":selected").val();
	if (attrib === 'state') {
		$("#cnf-class").val(devobj.attributes.device_class);
		$("#cnf-state").val(devobj.state_topic);
	} else {
		$("#cnf-class").val(attrib);
		$("#cnf-state").val(attrib);
	}
	if (devobj.attributes.hasOwnProperty(attrib+'_unit')) {
		$("#cnf-unit").val(devobj.attributes[attrib+'_unit']);
	} else {
		$("#cnf-unit").val(devobj.attributes.unit_of_measurement);
	}
	$("#test-state").val(devobj.attributes[attrib]);
	let url = 'i/s48-'+$("#cnf-class").val()+'.png';
	if (devobj.type === "alarm_control_panel") {
		url = 'i/d48-acp-'+devobj.attributes.state+'.png';
	} else
	if (devobj.type === "binary_sensor") {
		url = 'i/d48-'+devobj.attributes.device_class+'-'+devobj.attributes.state+'.png';
	} else
	if (devobj.type === "weather" && attrib === 'state') {
		url = 'i/d48-'+devobj.attributes.state+'.png';
	} else
	if (devobj.type === 'switch' || devobj.type === 'light' || devobj.type === 'cover' || devobj.type === 'lock') {
		url = 'i/d48-'+devobj.type+'-'+devobj.attributes.state+'.png';
	}
	$.ajax({
		url: url,
		headers: { "Cache-Control": "no-cache" }
	}).done(function(){
		$("#test-icon").attr('src', url);
	}).fail(function( jqXHR, textStatus ) {
	});			
}
