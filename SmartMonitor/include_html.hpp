const char html_header[] PROGMEM = R"rawliteral(
<div class="row col-md-12 pb-2">
  <div class="form-group  col-sm-1">
    <div class="input-group">
      <span class="input-group-text">Col.</span>
      <input type="number" class="form-control" name="col" value="%CNFCOL%">
    </div>
  </div>
  <div class="form-group  col-sm-1">
    <div class="input-group">
      <span class="input-group-text">Lig.</span>
      <input type="number" class="form-control" name="row" value="%CNFROW%">
    </div>
  </div>
  <div class="form-group col-sm-9">
    <div id="mqttloading" class="float-end"></div>
  </div>
  <div class="form-group col-sm-1">
    <div><button type="button" class="btn btn-warning btn-sm bi-search" onclick="initMqtt(this)">&nbsp;Charge Mqtt</button></div>
  </div>
</div>
<div id="conf_page" class="row">chargement...</div>
<div class="col-md-12 mt-2">
  <div class="input-group justify-content-end">
    <button type="button" class="btn btn-info bi-plus-circle" id="newpage" onclick="addpage(this)">&nbsp;Nouvelle page</button>
  </div>
</div>
)rawliteral";

const char html_page[] PROGMEM = R"rawliteral(
<div class="col-md-1 mt-2">
  <div class="input-group">
    <span class="input-group-text">id</span>
    <input type="text" class="form-control" name="pages_id_#P#" value="%CNFID%" readonly>
  </div>
</div>
<div class="col-md-11 mt-2">
  <div class="row pb-1">
    <div class="col-sm-2">
      <div class="input-group">
        <span class="input-group-text">Titre</span>
        <input type="text" class="form-control" name="pages_title_#P#" value="%CNFTITLE%">
      </div>
    </div>
    <div class="col-sm-2">
      <div class="input-group">
        <label class="input-group-text">Type</label>
        <select class="form-select" name="pages_type_#P#" >
          <option value="0" %CNFP_T0%>normalPage</option>
          <option value="1" %CNFP_T1%>keypadPage</option>
        </select>
      </div>
    </div>
  </div>
<table class="table table-borderless"><thead>
<tr><th scope="col" class="col-2">Titre</th><th scope="col" class="col-1">Action</th><th scope="col">Mqtt</th><th scope="col" class="col-1">Domaine</th><th scope="col" class="col-1">Classe</th><th scope="col" class="col-1">Unité</th><th scope="col" class="col-1">Page</th></tr>
</thead>
<tbody>%GENITEMS%</tbody>
</table>
</div>
)rawliteral";

const char html_item[] PROGMEM = R"rawliteral(
      <td><input type="text" class="form-control" name="pages_label_#P#_#I#" value="%CNFLABEL%"></td>
      <td>
        <select class="form-select" name="pages_action_#P#_#I#" >
        <option value="0" %CNFI_A0%>Rien</option>
        <option value="1" %CNFI_A1%>HAtrigger</option>
        <option value="2" %CNFI_A2%>changePage</option>
        <option value="3" %CNFI_A3%>Display</option>
        <option value="4" %CNFI_A4%>Command</option>
        </select>
      </td>
      <td>
        <div class="input-group">
          <button class="btn btn-warning btn-sm" type="button" name="btn_mqtt_#P#_#I#" data-bs-toggle="modal" data-bs-target="#myModal" data-source="#P#_#I#" disabled>...</button>
          <input type="text" class="form-control" name="pages_mqtt_#P#_#I#" value="%CNFMQTT%" %CNFI_MQTTOFF%></td>
        </div>
      </td>
      <td><input class="form-control" name="pages_source_#P#_#I#" value="%CNFSRC%" %CNFI_SRCOFF%></td>
      <td><input class="form-control" name="pages_icon_#P#_#I#" value="%CNFICON%" %CNFI_ICONOFF%></td>
      <td><input class="form-control" name="pages_unit_#P#_#I#"  value="%CNFUNIT%"%CNFI_UNITOFF%></td>
      <td><select class="form-select" name="pages_page_#P#_#I#" placeholder="%CNFPAGE%" %CNFI_PAGEOFF%></select></td>
)rawliteral";

const char html_hardware[] PROGMEM = R"rawliteral(
<div class="row pt-1">
<div class="col-md-1">
  <div class="input-group">
    <label class="input-group-text">pin</label>
    <!-- input type="number" class="form-control" name="gpio_pin_#H#" value="%CNFPIN%" -->
    <select class="form-select" name="gpio_pin_#H#" >
      <option value="-1" %CNFH_Pnone%></option>
      %CNFH_PINLIST%
    </select>
  </div>
</div>
<div class="col-md-5">
  <div class="input-group">
    <label class="input-group-text">Nom</label>
    <input type="text" class="form-control" name="gpio_name_#H#" value="%CNFNAME%">
  </div>
</div>
<div class="col-sm-3">
  <div class="input-group">
    <label class="input-group-text">Type</label>
    <select class="form-select" name="gpio_type_#H#" >
      <option value="0" %CNFH_T0%>noIO</option>
      <option value="1" %CNFH_T1%>buzzer 1 Pulse</option>
      <option value="2" %CNFH_T2%>buzzer PWM</option>
      <option value="3" %CNFH_T3% disabled>buzzer I2S</option>
      <option value="4" %CNFH_T4%>binarySensor</option>
      <option value="5" %CNFH_T5%>numberSensor</option>
      <option value="6" %CNFH_T6% disabled>stateSensor</option>
      <option value="7" %CNFH_T7% disabled>tagScanner</option>
      <option value="8" %CNFH_T8% disabled>switchOutput</option>
      <option value="9" %CNFH_T9% disabled>lockOutput</option>
      <option value="10" %CNFH_T10% disabled>lightOutput</option>
    </select>
  </div>
</div>
<div class="col-md-3">
  <div class="input-group flex-nowrap">
    <span class="input-group-text">Y = X *</span>
    <input type="text" class="form-control" name="gpio_coefa_#H#" %CNFH_COEFOFF%>
    <span class="input-group-text">+</span>
    <input type="text" class="form-control" name="gpio_coefb_#H#" %CNFH_COEFOFF%>
  </div>
</div>
</div>
)rawliteral";
