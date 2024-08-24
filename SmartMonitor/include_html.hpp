const char html_header[] PROGMEM = R"rawliteral(
<div id="conf_page" class="row">loading...</div>
)rawliteral";

const char html_page[] PROGMEM = R"rawliteral(
<div class="col-1">
  <label>id
    <input type="text" name="pages_id_#P#" value="%CNFID%" readonly>
  </label>
</div>
<div class="col-2">
  <label>Title
    <input type="text" name="pages_title_#P#" value="%CNFTITLE%">
  </label>
</div>
<div class="col-2">
  <label>Type
    <select name="pages_type_#P#" >
      <option value="0" %CNFP_T0%>normalPage</option>
      <option value="1" %CNFP_T1%>keypadPage</option>
    </select>
  </label>
</div>
<table class="table table-borderless">
<thead>
<tr>
  <th scope="col" class="col-2">Title</th>
  <th scope="col" class="col-1">Action</th>
  <th scope="col">Mqtt</th>
  <th scope="col" class="col-1">Domain</th>
  <th scope="col" class="col-1">Class</th>
  <th scope="col" class="col-1">Unit</th>
  <th scope="col" class="col-1">Page</th>
</tr>
</thead>
<tbody>%GENITEMS%</tbody>
</table>
</div>
)rawliteral";

const char html_item[] PROGMEM = R"rawliteral(
<td><input type="text" name="pages_label_#P#_#I#" value="%CNFLABEL%"></td>
<td>
  <select name="pages_action_#P#_#I#" >
  <option value="0" %CNFI_A0%>None</option>
  <option value="1" %CNFI_A1%>HAtrigger</option>
  <option value="2" %CNFI_A2%>changePage</option>
  <option value="3" %CNFI_A3%>Display</option>
  <option value="4" %CNFI_A4%>Command</option>
  </select>
</td>
<td class="row">
    <a role="button" class="col-1 secondary" style="margin-bottom:var(--spacing)" href="#cancel" id="btn_mqtt_#P#_#I#" data-target="modalha" onclick="editHA(event, #P#, #I#)" disabled>☰</a>
    <div class="col-11"><input type="text" name="pages_mqtt_#P#_#I#" value="%CNFMQTT%" %CNFI_MQTTOFF%>
</td>
<td><input name="pages_source_#P#_#I#" value="%CNFSRC%" %CNFI_SRCOFF%></td>
<td><input name="pages_icon_#P#_#I#" value="%CNFICON%" %CNFI_ICONOFF%></td>
<td><input name="pages_unit_#P#_#I#"  value="%CNFUNIT%"%CNFI_UNITOFF%></td>
<td><select name="pages_page_#P#_#I#" placeholder="%CNFPAGE%" %CNFI_PAGEOFF%></select></td>
)rawliteral";

const char html_hardware[] PROGMEM = R"rawliteral(
<div class="row">
<div class="col-1">
    <label>pin</label>
    <!-- input type="number" class="form-control" name="gpio_pin_#H#" value="%CNFPIN%" -->
    <select name="gpio_pin_#H#" >
      <option value="-1" %CNFH_Pnone%></option>
      %CNFH_PINLIST%
    </select>
</div>
<div class="col-5">
    <label class="input-group-text">Name</label>
    <input type="text" class="form-control" name="gpio_name_#H#" value="%CNFNAME%">
</div>
<div class="col-3">
    <label class="input-group-text">Type</label>
    <select class="form-select" name="gpio_type_#H#" >
      <option value="0" %CNFH_T0%>noIO</option>
      <option value="1" %CNFH_T1%>buzzer 1 Pulse</option>
      <option value="2" %CNFH_T2%>buzzer PWM</option>
      <option value="5" %CNFH_T5%>Luminosity Sensor</option>
      <option value="10" %CNFH_T10%>binary Sensor</option>
      <option value="11" %CNFH_T11%>number Sensor</option>
    </select>
</div>
<div class="col-3">
  <div class="row">
    <label>Calibration</label>
    <div class="col-3">Y = X *</div>
    <div class="col-4"><input type="text" name="gpio_coefa_#H#" %CNFH_COEFOFF%></div>
    <div class="col-1">+</div>
    <div class="col-4"><input type="text" name="gpio_coefb_#H#" %CNFH_COEFOFF%></div>
  </div>
</div>
</div>
)rawliteral";
