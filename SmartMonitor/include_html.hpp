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
</div>
<div id="conf_page" class="row">chargement...</div>
<div class="col-md-12 mt-2">
  <div class="input-group justify-content-end">
    <button type="button" class="btn btn-info" id="newpage" onclick="addpage(this)">Nouvelle page</button>
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
  %GENITEMS%
</div>
)rawliteral";

const char html_item[] PROGMEM = R"rawliteral(
<div class="row pt-1">
  <div class="col-sm-2">
    <div class="input-group">
      <span class="input-group-text">Titre</span>
      <input type="text" class="form-control" name="pages_label_#P#_#I#" value="%CNFLABEL%">
    </div>
  </div>
  <div class="col-sm-2">
    <div class="input-group">
      <label class="input-group-text">Action</label>
      <select class="form-select" name="pages_action_#P#_#I#" >
        <option value="0" %CNFI_A0%>Rien</option>
        <option value="1" %CNFI_A1%>HAtrigger</option>
        <option value="2" %CNFI_A2%>changePage</option>
        <option value="3" %CNFI_A3%>Display</option>
        <option value="4" %CNFI_A4%>Command</option>
      </select>
    </div>
  </div>
  <div class="col-sm-5">
    <div class="input-group">
      <span class="input-group-text">Mqtt</span>
      <input type="text" class="form-control" name="pages_mqtt_#P#_#I#" value="%CNFMQTT%" %CNFI_MQTTOFF%>
    </div>
  </div>
  <div class="col-sm-2">
    <div class="input-group">
      <label class="input-group-text">Type</label>
      <select class="form-select" name="pages_datatype_#P#_#I#" defval="%CNFTYPE%" %CNFI_TYPEOFF%>
      </select>
    </div>
  </div>
  <div class="col-sm-1 px-0">
    <div class="input-group">
      <span class="input-group-text">Page</span>
      <select class="form-select" name="pages_page_#P#_#I#" placeholder="%CNFPAGE%" %CNFI_PAGEOFF%>
      </select>
    </div>
  </div>
</div>
)rawliteral";

const char html_hardware[] PROGMEM = R"rawliteral(
<div class="row pt-1">
<div class="col-md-4">
  <div class="input-group">
    <label class="input-group-text">pin</label>
    <!-- input type="number" class="form-control" name="gpio_pin_#H#" value="%CNFPIN%" -->
    <select class="form-select" name="gpio_pin_#H#" >
      <option value="0" %CNFH_P0%></option>
      %CNFH_PINLIST%
    </select>
  </div>
</div>
<div class="col-md-4">
  <div class="input-group">
    <label class="input-group-text">Nom</label>
    <input type="text" class="form-control" name="gpio_name_#H#" value="%CNFNAME%">
  </div>
</div>
<div class="col-sm-4">
  <div class="input-group">
    <label class="input-group-text">Type</label>
    <select class="form-select" name="gpio_type_#H#" >
      <option value="0" %CNFH_T0%>binarySensor</option>
      <option value="1" %CNFH_T1% disabled>numberSensor</option>
      <option value="2" %CNFH_T2% disabled>stateSensor</option>
      <option value="3" %CNFH_T3% disabled>tagScanner</option>
      <option value="4" %CNFH_T4% disabled>switchOutput</option>
      <option value="5" %CNFH_T5% disabled>lockOutput</option>
      <option value="6" %CNFH_T6% disabled>lightOutput</option>
    </select>
  </div>
</div>
</div>
)rawliteral";

const char js_loadselect[] PROGMEM = R"rawliteral(
)rawliteral";
