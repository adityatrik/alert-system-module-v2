//=========================================
//HTML + CSS + JavaScript codes for webpage
//=========================================
extern String myString;
const char WIFI_MENU[] =
R"=====(
<!DOCTYPE html>
<html>
  <head>
    <style>
      /* CSS for login page */
      body {
        background-color: #f2f2f2;
        font-family: Arial, sans-serif;
        margin: 0;
        padding: 0;
      }
      .switch {
        position: relative;
        display: inline-block;
        width: 60px;
        height: 34px;
      }

      .switch input {
        opacity: 0;
        width: 0;
        height: 0;
      }
      .slider {
        position: absolute;
        cursor: pointer;
        top: 0;
        left: 0;
        right: 0;
        bottom: 0;
        background-color: #ccc;
        -webkit-transition: 0.4s;
        transition: 0.4s;
      }

      .slider:before {
        position: absolute;
        content: "";
        height: 26px;
        width: 26px;
        left: 4px;
        bottom: 4px;
        background-color: white;
        -webkit-transition: 0.4s;
        transition: 0.4s;
      }
      input:checked + .slider {
        background-color: #2196f3;
      }

      input:focus + .slider {
        box-shadow: 0 0 1px #2196f3;
      }

      input:checked + .slider:before {
        -webkit-transform: translateX(26px);
        -ms-transform: translateX(26px);
        transform: translateX(26px);
      }
      /* Rounded sliders */
      .slider.round {
        border-radius: 34px;
      }

      .slider.round:before {
        border-radius: 50%%;
      }
      .login-container {
        width: 300px;
        margin: 50px auto;
        background-color: #fff;
        padding: 20px;
        border-radius: 5px;
        box-shadow: 0px 0px 10px #ccc;
      }
      .login-form {
        display: flex;
        flex-wrap: wrap;
        align-items: center;
        justify-content: center;
      }
      .form-group {
        width: 100%%;
        margin-bottom: 10px;
      }
      .form-control {
        width: 100%%;
        padding: 12px 20px;
        margin: 8px 0;
        box-sizing: border-box;
        border: 2px solid #ccc;
        border-radius: 4px;
      }
      .form-control:focus {
        border: 2px solid #555;
      }
      .form-submit {
        width: 100%%;
        background-color: #4caf50;
        color: white;
        padding: 14px 20px;
        margin: 8px 0;
        border: none;
        border-radius: 4px;
        cursor: pointer;
      }
      .form-submit:hover {
        background-color: #45a049;
      }
      .form-label {
        font-size: 14px;
        font-weight: bold;
        margin-bottom: 8px;
      }
    </style>
  </head>
  <body>
    <div class="login-container"  action='/login' method='POST'>
        <h1 style="color:#45a049;text-align: center;">ALERT SYSTEM MODULE</h1>
        <h3 style="text-align: center;">ID:ASM2301002</h3>
      <form class="login-form">
        <div class="form-group">
          <label class="form-label" for="username">SSID</label>
          <input
            type="text"
            name='SSID'
            class="form-control"
            id="ssid"
            placeholder="Masukkan SSID WiFi"
            value="%s"
          />
        </div>
        <div class="form-group">
          <label class="form-label" for="password">PASSWORD</label>
          <input
            type="text"
            name='PASSWORD'
            class="form-control"
            id="password"
            placeholder="Masukkan password WiFi"
            value="%s"
          />
        </div>
        <div class="form-group">
          <label class="form-label" for="password">IP ADDRESS</label>
          <input
            type="text"
            name='IP'
            class="form-control"
            id="ip_addr"
            placeholder="Masukkan IP Address"
            value="%s"
          />
        </div>
        <div class="form-group">
          <label class="form-label" for="password">NETMASK</label>
          <input
            type="text"
            name='NETMASK'
            class="form-control"
            id="netmask"
            placeholder="Masukkan Netmask"
            value="%s"
          />
        </div>
        <div class="form-group">
          <label class="form-label" for="password">GATEWAY</label>
          <input
            type="text"
            name='GATEWAY'
            class="form-control"
            id="gateway"
            placeholder="Masukkan Gateway"
            value="%s"
          />
        </div>
        <div class="form-group">
          <label class="form-label" for="password">DNS</label>
          <input
            type="text"
            name='DNS'
            class="form-control"
            id="dns"
            placeholder="Masukkan DNS"
            value="%s"
          />
        </div>
        <button type="submit" name='SUBMIT' class="form-submit">Simpan</button>
      </form>
      <a href="/"><button type="submit" name='SUBMIT' class="form-submit">Kembali</button></a>      
    </div>
  </body>
</html>

)=====";