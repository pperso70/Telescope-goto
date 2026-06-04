extern const char webpageCont[] PROGMEM =
R"=====(<!doctype html>
<html>

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP telescope</title>
    <meta name="google" content="notranslate">
    <link rel="icon"
        href="data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 16 16'%3E%3Cpath d='M14 4l-1-2L2 5l1 2L14 4zM1 7l4.5 1L7 13H5v2h6v-2H9L7.5 8 13 6.5' fill='%232563eb' stroke='%231e40af' stroke-width='0.8' stroke-linejoin='round'/%3E%3C/svg%3E"
        type="image/svg+xml">
    <link rel="alternate icon"
        href="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAACXBIWXMAAAsTAAALEwEAmpwYAAABSElEQVR4nGNgoBAw4pL4X8/KwFDPylDPysxQz8QE5NcxMNSzMhHWhE0zsgH1rPX1rAyX61kZLtezMpyvZ2X4D8L1rAyP61kZzhPShGzz/3pWhssMDAwM9awMl+pZGR7XszL8h+J6VoaL9awMZ/BpxrD5fz0rw1UGBob/9awMl+tZGZ7UszL8g+J6VobL9awMp3FpxrAZKngFqPgJkVEKkz9fz8pwCptmrDYDFV+tZ2V4Ws/K8BeL/OV6VoYT2DQzYvgZqPBaPSvDs3pWhn84NC/DphnDgHpWhusMDAxP61kZ/uKQv1TPynAUXTMjhp/rWRluMDAwPEPXjEUe7GcGhgcMDNgBTnWsBoZnDAz/kcMYxK5jNUC1H6qwnpXhGa6YR9bMyPCMgQEj3PFpZoTRDM8YGLDSBS7NVEkZAE0YvYa4p4p3AAAAAElFTkSuQmCC"
        type="image/png">

    <!--commentaire HTML-->
    <!------------------------------------------CSS----------------------------------------->
    <style>
        body {
            background-color: #000;
            /* Couleur de fond par défaut en mode clair */
        }

        html {
            font-family: Arial;
            display: inline-block;
            text-align: center;
            line-height: 1px
        }

        a {
            color: #EFEFEF;
            text-decoration: underline
        }

        h2 {
            font-size: 1.5rem
        }

        span {
            color: white
        }

        p {
            font-size: 3.0rem
        }

        input {
            color: white;
            background: #080a72;
            font-size: 1.4rem;
            font-weight: bold
        }

        .bouton {
            font-size: 15px;
            width: 100px;
            height: 70px;
            padding-bottom: 10px;
            display: block;
            font-weight: bold;
            color: white;
            background: #6e1616
        }


        #aff_buttons_cam {
            display: flex;
            flex-direction: row;
            /*padding: 10px;*/
            margin-top: 10px;
            margin-bottom: 10px;
        }

        .gros_bouton {
            font-size: 15px;
            width: 90px;
            height: 50px;
            padding-bottom: 10px;
            display: block;
            font-weight: bold;
            color: white;
            background: #6e1616
        }

        .boutonMvmt {
            font-size: 15px;
            width: 150px;
            height: 70px;
            padding-bottom: 10px;
            font-weight: bold;
            color: white;
            background: #6e1616
        }

        button:hover {
            background: #501617
        }

        button:active {
            background: #4e0305
        }

        .Cmdes {
            /*jaune*/
            position: absolute;
            background: #6b6801;
            top: 10px;
            left: 5px
        }

        .Pos_Init {
            /*vert*/
            display: flex;
            flex-direction: column;
            position: absolute;
            background: #0d3f0d;
            top: 10px;
            left: 105px
        }

        .Pos_Vitesse {
            /*rose*/
            display: flex;
            flex-direction: column;
            position: absolute;
            background: #72565b;
            top: 10px;
            left: 205px
        }

        .Pos_Mvments {
            /*rouge*/
            position: absolute;
            background: #331111;
            top: 340px;
            left: 125px;
            width: 400px
        }

        .Pos_Divers {
            /*cyan*/
            column-count: 2;
            position: absolute;
            background: #1b645e;
            top: 10px;
            left: 307px
        }

        .Pos_Mes_Analog {
            /*vert*/
            background: #0d3f0d;
            position: absolute;
            top: 10px;
            left: 540px;
            width: 140px;
            height: 450px;
            text-align: left;
            padding-left: 5px
        }


        .Pos_Mes_Analog_cam {
            display: flex;
            flex-direction: row;
            color: darkred;
            font-size: 40 px;
            padding-left: 5px
        }

        .Pos_Cmd_Analog {
            /*bleu*/
            display: flex;
            flex-direction: column;
            position: absolute;
            background: #080a72;
            top: 460px;
            left: 540px;
            width: 145px;
            height: 172px
        }

        .Pos_Gris1 {
            /*gris1*/
            position: absolute;
            background: grey;
            top: 260px;
            left: 140px;
            width: 180px
        }

        .Pos_Gris2 {
            /*gris2*/
            position: absolute;
            background: grey;
            top: 260px;
            left: 340px;
            width: 180px
        }
        
        #cadre {
            position: absolute;
            top: 630px;
            left: 5px;
            width: 680px;
            height: 100px
        }
        
        #live {
            display: none;
            position: absolute;
            top: 0px;
            left: 700px;
            width: 780px;
            height: 550px;
        }

        body.image-visible {
            min-width: 1500px;
            /* 700 + 780 + marge */
        }

        #menu {
            flex-wrap: nowrap;
            color: #EFEFEF;
            width: 380px;
            background: #363636;
            padding: 8px;
            border-radius: 4px;
            margin-top: -10px;
            margin-right: 10px
        }

        .input-group {
            display: flex;
            flex-wrap: nowrap;
            line-height: 22px;
            margin: 5px 0
        }

        .input-group>label {
            display: inline-block;
            padding-right: 10px;
            min-width: 47%
        }

        .input-group input,
        .input-group select {
            flex-grow: 1
        }

        .range-max,
        .range-min {
            display: inline-block;
            padding: 0 5px
        }

        input[type=range] {
            -webkit-appearance: none;
            width: 0;
            height: 22px;
            background: #363636;
            cursor: pointer;
            margin: 0
        }

        input[type=range]:focus {
            outline: 0
        }

        input[type=range]::-webkit-slider-runnable-track {
            width: 100%;
            height: 2px;
            cursor: pointer;
            background: #EFEFEF;
            border-radius: 0;
            border: 0 solid #EFEFEF
        }

        input[type=range]::-webkit-slider-thumb {
            border: 1px solid rgba(0, 0, 30, 0);
            height: 22px;
            width: 22px;
            border-radius: 50px;
            background: #6e1616;
            cursor: pointer;
            -webkit-appearance: none;
            margin-top: -11.5px
        }

        input[type=range]:focus::-webkit-slider-runnable-track {
            background: #EFEFEF
        }

        input[type=range]::-moz-range-track {
            width: 100%;
            height: 2px;
            cursor: pointer;
            background: #EFEFEF;
            border-radius: 0;
            border: 0 solid #EFEFEF
        }

        input[type=range]::-moz-range-thumb {
            border: 1px solid rgba(0, 0, 30, 0);
            height: 22px;
            width: 22px;
            border-radius: 50px;
            background: #6e1616;
            cursor: pointer
        }

        input[type=range]::-ms-track {
            width: 100%;
            height: 2px;
            cursor: pointer;
            background: 0 0;
            border-color: transparent;
            color: transparent
        }

        input[type=range]::-ms-fill-lower {
            background: #EFEFEF;
            border: 0 solid #EFEFEF;
            border-radius: 0
        }

        input[type=range]::-ms-fill-upper {
            background: #EFEFEF;
            border: 0 solid #EFEFEF;
            border-radius: 0
        }

        input[type=range]::-ms-thumb {
            border: 1px solid rgba(0, 0, 30, 0);
            height: 22px;
            width: 22px;
            border-radius: 50px;
            background: #6e1616;
            cursor: pointer;
            height: 2px
        }

        input[type=range]:focus::-ms-fill-lower {
            background: #EFEFEF
        }

        input[type=range]:focus::-ms-fill-upper {
            background: #363636
        }

        input[type=text] {
            border: 1px solid #363636;
            font-size: 14px;
            height: 20px;
            margin: 1px;
            outline: 0;
            border-radius: 5px
        }

        #popup {
            display: none;
            position: fixed;
            top: 15%;
            left: 50%;
            transform: translateX(-50%);
            background-color: #f8f8f8;
            border: 1px solid #ccc;
            padding: 20px;
            border-radius: 12px;
            box-shadow: 0 0 15px rgba(0, 0, 0, 0.3);
            z-index: 1000;
            width: 400px;
        }

        #overlay {
            display: none;
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background-color: rgba(0, 0, 0, 0.4);
            z-index: 999;
        }

        label {
            display: block;
            margin-top: 10px;
        }

        input {
            width: 80px;
        }

        .dms-inputs {
            display: flex;
            gap: 5px;
            align-items: center;
        }
    </style>
</head>

<!----------------------------------------Body + Script----------------------------------------->

<body>
    <div class="Cmdes">
        <h2>Cmdes</h2>
        <button onclick="sendCommand('parking nord')" class="bouton">parking nord</button>
        <button onclick="sendCommand('parking sud')" class="bouton">parking sud</button>
        <button onclick="sendCommand('suivit')" class="bouton">suivit</button>
        <button onclick="sendCommand('pointage')" class="bouton">pointage</button>
        <button onclick="sendCommand('arret')" class="bouton">arret</button>
        <button onclick="sendCommand('EQ')" class="bouton">EQ</button>
        <button onclick="openPopup()" class="bouton">Coord. GPS</button>
    </div>

    <div class="Pos_Init">
        <h2>Init</h2>
        <button onclick="sendCommand('init parking nord')" class="bouton">init parking nord</button>
        <button onclick="sendCommand('init parking sud')" class="bouton">init parking sud</button>
        <button onclick="sendCommand('raz_delta')" class="bouton">raz delta</button>
    </div>

    <div class="Pos_Vitesse">
        <h2> vitesse </h2>
        <button onclick="sendCommand('lent')" class="bouton">lent</button>
        <button onclick="sendCommand('moyen')" class="bouton">moyen</button>
        <button onclick="sendCommand('rapide')" class="bouton">rapide</button>
    </div>

    <div class="Pos_Mvments">
        <h2>mouvements</h2>
        <button onmousedown="startCommand('haut')" onmouseup="stopCommand()" onmouseleave="stopCommand()"
            class="boutonMvmt">haut</button>
        <h5></h5>
        <button onmousedown="startCommand('gauche')" onmouseup="stopCommand()" onmouseleave="stopCommand()"
            class="boutonMvmt">gauche</button>
        <button onmousedown="startCommand('droit')" onmouseup="stopCommand()" onmouseleave="stopCommand()"
            class="boutonMvmt">droit</button>
        <h5></h5>
        <button onmousedown="startCommand('bas')" onmouseup="stopCommand()" onmouseleave="stopCommand()"
            class="boutonMvmt">bas</button>
    </div>

    <div id="cadre">
        <section id="aff_buttons_cam">
            <button onclick="sendCommandCam('GetStill')" id="get-still" class="gros_bouton">Get Still</button>
            <button onclick="sendCommand('DownFast')" id="down_fast" class="gros_bouton">Down Fast</button>
            <button onclick="sendCommand('Down')" id="down" class="gros_bouton">Down</button>
            <button onclick="sendCommand('Up')" id="up" class="gros_bouton">Up</button>
            <button onclick="sendCommand('UpFast')" id="up_fast" class="gros_bouton">Up Fast</button>
            <button onclick="sendCommand('Excur')" id="excur" class="gros_bouton">Excur</button>
            <button onclick="sendCommand('Stop')" id="stop" class="gros_bouton">Stop</button>
            <button onclick="sendCommand('Raz5000')" id="Raz" class="gros_bouton">Raz 5000</button>
            <button onclick="toggleImage()" id="btnImage" class="gros_bouton">Image</button>
        </section>

        <div class="Pos_Mes_Analog_cam">
            <span>position act: </span> <span id="PositionCam">0</span>
            <span>&nbsp;&nbsp;&nbsp;&nbsp;</span> <!-- 5 non-breaking spaces -->
            <span>RSSI: </span> <span id="Cam_Val2">0</span>
            <span>&nbsp;&nbsp;&nbsp;&nbsp;</span>
            <span>T cyle: </span> <span id="temps_de_cycle">0</span>
            <span>&nbsp;&nbsp;&nbsp;&nbsp;</span>
            <span> lampe val: </span> <span id="Cam_Val3">0</span>
        </div>


        <div class="input-group" id="lamp-group" title="Flashlight LED">
            <label for="lamp">Light</label>
            <div class="range-min">Off</div>
            <input type="range" id="lamp" min="0" max="99" value="0" class="action-setting">
            <div class="range-max">Full&#9888;</div>
        </div>

    </div>
    <img id='live' src='' width="780" height="550">

    <div class="Pos_Divers">
        <h2>divers</h2>
        <div>
            <button onclick="sendCommand('cherche')" class="bouton">cherche</button>
            <button onclick="sendCommand('360')" class="bouton">360</button>
            <button onclick="sendCommand('zenith')" class="bouton">zenith</button>
        </div>
        <div>
            <button onclick="sendCommand('tour')" class="bouton">tour</button>
            <button onclick="sendCommand('Aff')" class="bouton">Aff</button>
            <button onclick="sendCommand('+ xx')" class="bouton">+ xx</button>
        </div>
    </div>

    <div class="Pos_Cmd_Analog">
        <h2>Input</h2>
        <input type="text" onchange="FuncSend(this)" id="ana1">
        <input type="text" onchange="FuncSend(this)" id="ana2">
        <input type="text" onchange="FuncSend(this)" id="ana3">
        <input type="text" onchange="FuncSend(this)" id="ana4">
        <input type="text" onchange="FuncSend(this)" id="ana5">
        <input type="text" onchange="FuncSend(this)" id="ana6">
    </div>


    <div class="Pos_Mes_Analog">
        <h2>Output</h2>
        <span>Tcycle</span> <span id="Tcycle">0</span><br>
        <canvas id="tcycleCanvas" width="90" height="50"
            style="border:1px solid #444; background:#000; margin-top:4px;"></canvas>
        <h5></h5><span>Lat</span> <span id="lati">0</span>
        <h5></h5><span>Long</span> <span id="long">0</span>
        <h5></h5><span>RA</span> <span id="RA">0</span>
        <h5></h5><span>DEC</span> <span id="DEC">0</span>
        <h5></h5><span>AziRefGi</span> <span id="AziRefGi">0</span>
        <h5></h5><span>AziActGi</span> <span id="AziActGi">0</span>
        <h5></h5><span>AltRefGi</span> <span id="AltRefGi">0</span>
        <h5></h5><span>AltActGi</span> <span id="AltActGi">0</span>
        <h5></h5><span>T</span> <span id="Heure">0</span> <span>H</span> <span id="Mn">0</span> <span>Mn</span> <span id="Sec">0</span><span>S</span>
        <h5></h5><span>Rayon</span> <span id="Rayon">0</span>
        <h5></h5><span>AziGap</span> <span id="AziGap">0</span>
        <h5></h5><span>AltGap</span> <span id="AltGap">0</span>
        <h5></h5><span>RSSI</span> <span id="RSSI">0</span>
        <h5></h5><span>PosCam</span> <span id="Position">0</span>
        <h5></h5><span>-</span>
    </div>

    <div class="Pos_Gris1" style="height: 80px;width: 175px;">
        <h5></h5><span>mode </span> <span id="Mode">0</span>
        <h5></h5><span>Azi ref</span> <span id="AziRef">0</span>
        <h5></h5><span>Azi act</span> <span id="AziAct">0</span>

    </div>

    <div class="Pos_Gris2" style="height: 80px;width: 179px;">
        <h5></h5><span>Vit.</span> <span id="Dratio">0</span>
        <h5></h5><span>Alt ref</span> <span id="AltRef">0</span>
        <h5></h5><span>Alt act</span> <span id="AltAct">0</span>
    </div>




    <div id="overlay"></div>
    <div id="popup">
        <h3>Latitude et Longitude</h3>

        <label>Latitude (° décimal) :
            <input type="number" step="0.0001" id="latDecimal" onchange="updateLatDMS()">
        </label>
        <div class="dms-inputs">
            <input type="number" id="latDeg" onchange="updateLatDecimal()">°
            <input type="number" id="latMin" onchange="updateLatDecimal()">′
            <input type="number" step="0.01" id="latSec" onchange="updateLatDecimal()">″
        </div>

        <label>Longitude (° décimal) :
            <input type="number" step="0.0001" id="lonDecimal" onchange="updateLonDMS()">
        </label>
        <div class="dms-inputs">
            <input type="number" id="lonDeg" onchange="updateLonDecimal()">°
            <input type="number" id="lonMin" onchange="updateLonDecimal()">′
            <input type="number" step="0.01" id="lonSec" onchange="updateLonDecimal()">″
        </div>

        <br>
        <button onclick="sendLatLonAndClose()">Fermer</button>
    </div>

    <!---------------------------------------javascript---------------------------------------->
    <script>
        //document.body.style.minWidth = (700 + img.offsetWidth + 20) + "px";

        const RANGE_INPUT_ID = 'lamp';
        let websock;      // WebSocket pour telemetrie
        let websockCam;   // WebSocket unifié pour la caméra (commandes + stream)
        let coordsInitialized;
        let globalData;  // Variable au scope global

        // ===== Courbe Tcycle =====
        const tcycleCanvas = document.getElementById("tcycleCanvas");
        const tctx = tcycleCanvas.getContext("2d");

        const TCYCLE_MAX_POINTS = 90;

        // Échelle FIXE
        const TCYCLE_MIN = 2900;  // 2.9 ms
        const TCYCLE_MAX = 4500;  // 4.5 ms

        // Seuil alarme
        const TCYCLE_ALARM = 5000;

        let tcycleHistory = [];

        function addTcyclePoint(value) {
            if (isNaN(value)) return;

            tcycleHistory.push(value);

            if (tcycleHistory.length > TCYCLE_MAX_POINTS) {
                tcycleHistory.shift();
            }

            drawTcycleGraph();
        }

        function drawTcycleGraph() {
            const w = tcycleCanvas.width;
            const h = tcycleCanvas.height;

            tctx.clearRect(0, 0, w, h);

            if (tcycleHistory.length < 2) return;

            // Fond
            tctx.fillStyle = "#000";
            tctx.fillRect(0, 0, w, h);

            // Grille bas
            tctx.strokeStyle = "#222";
            tctx.beginPath();
            tctx.moveTo(0, h - 0.5);
            tctx.lineTo(w, h - 0.5);
            tctx.stroke();

            // Dernière valeur
            const lastValue = tcycleHistory[tcycleHistory.length - 1];

            // Couleur selon seuil
            tctx.strokeStyle = (lastValue > TCYCLE_ALARM) ? "#ff3333" : "#00ff88";
            tctx.lineWidth = 1;
            tctx.beginPath();

            tcycleHistory.forEach((val, i) => {
                const x = (i / (TCYCLE_MAX_POINTS - 1)) * w;

                // Clamp à l’échelle fixe
                const clamped = Math.max(TCYCLE_MIN, Math.min(TCYCLE_MAX, val));

                const y = h - ((clamped - TCYCLE_MIN) / (TCYCLE_MAX - TCYCLE_MIN)) * h;

                if (i === 0) tctx.moveTo(x, y);
                else tctx.lineTo(x, y);
            });

            tctx.stroke();

            // Ligne seuil visuelle (optionnelle mais utile)
            const yAlarm = h - ((TCYCLE_ALARM - TCYCLE_MIN) / (TCYCLE_MAX - TCYCLE_MIN)) * h;
            if (yAlarm >= 0 && yAlarm <= h) {
                tctx.strokeStyle = "#552222";
                tctx.setLineDash([2, 2]);
                tctx.beginPath();
                tctx.moveTo(0, yAlarm);
                tctx.lineTo(w, yAlarm);
                tctx.stroke();
                tctx.setLineDash([]);
            }
        }

        function toggleImage() {
            const img = document.getElementById("live");

            if (img.style.display === "none" || img.style.display === "") {
                img.style.display = "block";
                document.body.classList.add("image-visible");
            } else {
                img.style.display = "none";
                document.body.classList.remove("image-visible");
            }
        }

        function InitWebSocket() {
            //const ws_url = 'ws://{IP_ADDRESS_TELE}:81/'; // Remplace par IP réelle si ce n’est pas dynamique
            const ws_url = 'ws://teles.local:81/'; // Remplace par IP réelle si ce n’est pas dynamique
            

            function connect() {
                websock = new WebSocket(ws_url);

                websock.onopen = function () {
                    console.log("✅ WebSocket connecté");
                };

                websock.onmessage = function (event) {
                    handleWebSocketMessage(event);
                };

                websock.onerror = function (error) {
                    console.error("WebSocket error:", error);
                    websock.close(); // Force la fermeture pour déclencher onclose()
                };

                websock.onclose = function () {
                    console.warn("🔄 WebSocket fermé, tentative de reconnexion...");
                    setTimeout(connect, 2000); // Reconnexion après 2 secondes
                };
            }

            connect(); // Lancer la première tentative
        }


        function InitWebSocketCamera() {
            //websockCam = new WebSocket('ws://192.168.1.71:82/'); // WebSocket unifié pour la caméra
            //websockCam = new WebSocket('ws://{IP_ADDRESS_CAM}:82/'); // WebSocket unifié pour la caméra
              websockCam = new WebSocket('ws://camera.local:82/'); // WebSocket unifié pour la caméra

            websockCam.binaryType = 'arraybuffer';

            websockCam.onmessage = function (event) {
                // Vérifie si les données sont binaires (image) ou JSON (commandes)
                if (event.data instanceof ArrayBuffer) {
                    handleVideoStream(event.data);
                } else {
                    handleCameraMessage(event.data);
                }
            };

            websockCam.onerror = function (error) {
                console.error("WebSocket Camera error:", error);
            };
        }


        function handleVideoStream(data) {
            const bytes = new Uint8Array(data);
            const binary = Array.from(bytes).map(byte => String.fromCharCode(byte)).join('');
            const img = document.getElementById('live');
            img.src = 'data:image/jpg;base64,' + window.btoa(binary);
        }

        function handleCameraMessage(data) {
            try {
                const parsedData = JSON.parse(data);
                if (parsedData) {
                    document.getElementById('temps_de_cycle').textContent = parsedData.temps_de_cycle;
                    //document.getElementById('Cam_Val1').textContent = parsedData.position;
                    document.getElementById('Cam_Val2').textContent = parsedData.rssi;
                    document.getElementById('Cam_Val3').textContent = parsedData.lampVal;
                }
            } catch (error) {
                console.error("Erreur lors du parsing des données caméra:", error);
            }
        }

        function handleWebSocketMessage(event) {
            const data = JSON.parse(event.data);

            if (!data) return;

            updateDOMElements(data);
            updateRayonColor(data.rayon);
        }

        function updateDOMElements(data) {
            globalData = data;
            const elementsToUpdate = {
                lati: data.latitude,
                long: data.longitude,
                Tcycle: data.temps_de_cycle,
                RA: data.getRAdec,
                DEC: data.getDeclinationDec,
                AziRefGi: data.Setpoint_azi,
                AziActGi: data.encoder_azi,
                AltRefGi: data.Setpoint_alt,
                AltActGi: data.encoder_alt,
                Heure: data.heure,
                Mn: data.minute,
                Sec: data.second_ms,
                Dratio: data.Dratio,
                AziRef: data.consigne_azi,
                AltRef: data.consigne_alt,
                Mode: data.mode,
                AziAct: data.points_angle_azi,
                AltAct: data.points_angle_alt,
                Rayon: data.rayon,
                RSSI: data.WiFi_RSSI,
                AziGap: data.AziGap,
                AltGap: data.AltGap,
                Position: data.Position,
                PositionCam: data.Position
            };


            for (const [id, value] of Object.entries(elementsToUpdate)) {
                document.getElementById(id).textContent = value;
            }
            addTcyclePoint(Number(data.temps_de_cycle));
        }

        function updateRayonColor(rayon) {
            const element = document.getElementById('Rayon');
            element.style.color = rayon !== 0 ? 'red' : 'white';
        }

        function sendCommand(command) {
            console.log("Envoi de la commande :", command);
            console.log("Position :", Position);

            const message = JSON.stringify({ command });

            if (websock && websock.readyState === WebSocket.OPEN) {
                websock.send(message);
            } else {
                console.error("Connexion WebSocket non établie.");
            }
        }

        let intervalId;


        function startCommand(direction) {
            sendCommand(direction); // premier envoi immédiat
            intervalId = setInterval(() => {
                sendCommand(direction); // répéter tant que bouton enfoncé
            }, 200); // toutes les 200 ms
        }

        function stopCommand() {
            clearInterval(intervalId);
        }



        function sendCommandCam(command) {
            console.log("Envoi de la commande caméra :", command);
            const message = JSON.stringify({ command });

            if (websockCam && websockCam.readyState === WebSocket.OPEN) {
                websockCam.send(message);
            } else {
                console.error("Connexion WebSocket caméra non établie.");
            }
        }

        function FuncSend(inputElement) {
            const command = `${inputElement.id} ${inputElement.value}`;
            //console.log("inputElement.id:",inputElement.id);
            sendCommand(command);
        }

        // Gestionnaire d'événements pour le curseur de la lampe
        document.getElementById(RANGE_INPUT_ID).addEventListener('input', function () {
            const rangeValue = this.value;
            const varanaCam = `varanaCam1=${rangeValue}`;
            sendCommandCam(varanaCam);
        });

        // Gestionnaire pour la capture d'image
        document.getElementById('get-still').addEventListener('click', function () {
            const canvas = document.createElement('canvas');
            const img = document.getElementById('live');
            canvas.width = img.width;
            canvas.height = img.height;
            const ctx = canvas.getContext('2d');
            ctx.drawImage(img, 0, 0, canvas.width, canvas.height);
            const dataURL = canvas.toDataURL('image/jpeg');

            const a = document.createElement('a');
            a.href = dataURL;
            a.download = 'esp32cam_capture.jpg';
            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);
        });

        // Initialisation des WebSockets
        InitWebSocket();
        InitWebSocketCamera();

        function openPopup() {
            document.getElementById('popup').style.display = 'block';
            document.getElementById('overlay').style.display = 'block';

            // Initialiser seulement la première fois

            //if (!coordsInitialized) {
            if (!coordsInitialized && globalData) {
                console.log(globalData.latitudeDec);
                console.log(globalData.longitudeDec);
                document.getElementById('latDecimal').value = globalData.latitudeDec;
                document.getElementById('lonDecimal').value = globalData.longitudeDec;
                updateLatDMS();
                updateLonDMS();
                coordsInitialized = true;
            }
        }

        function closePopup() {
            document.getElementById('popup').style.display = 'none';
            document.getElementById('overlay').style.display = 'none';
        }



        function decimalToDMS(value) {
            const sign = value < 0 ? -1 : 1;
            const abs = Math.abs(value);
            const deg = Math.floor(abs);
            const minFloat = (abs - deg) * 60;
            const min = Math.floor(minFloat);
            const sec = (minFloat - min) * 60;
            return { deg: deg * sign, min, sec };
        }

        function dmsToDecimal(deg, min, sec) {
            const sign = deg < 0 ? -1 : 1;
            return sign * (Math.abs(deg) + min / 60 + sec / 3600);
        }

        function updateLatDMS() {
            const value = parseFloat(document.getElementById('latDecimal').value);
            if (isNaN(value)) return;
            const dms = decimalToDMS(value);
            document.getElementById('latDeg').value = dms.deg;
            document.getElementById('latMin').value = Math.floor(dms.min);
            document.getElementById('latSec').value = dms.sec.toFixed(2);
        }

        function updateLatDecimal() {
            const deg = parseFloat(document.getElementById('latDeg').value);
            const min = parseFloat(document.getElementById('latMin').value);
            const sec = parseFloat(document.getElementById('latSec').value);
            if (isNaN(deg) || isNaN(min) || isNaN(sec)) return;
            const dec = dmsToDecimal(deg, min, sec);
            document.getElementById('latDecimal').value = dec.toFixed(6);
        }

        function updateLonDMS() {
            const value = parseFloat(document.getElementById('lonDecimal').value);
            if (isNaN(value)) return;
            const dms = decimalToDMS(value);
            document.getElementById('lonDeg').value = dms.deg;
            document.getElementById('lonMin').value = Math.floor(dms.min);
            document.getElementById('lonSec').value = dms.sec.toFixed(2);
        }

        function updateLonDecimal() {
            const deg = parseFloat(document.getElementById('lonDeg').value);
            const min = parseFloat(document.getElementById('lonMin').value);
            const sec = parseFloat(document.getElementById('lonSec').value);
            if (isNaN(deg) || isNaN(min) || isNaN(sec)) return;
            const dec = dmsToDecimal(deg, min, sec);
            document.getElementById('lonDecimal').value = dec.toFixed(6);
        }

        function sendLatLonAndClose() {
            const latInput = document.getElementById('latDecimal');
            const lonInput = document.getElementById('lonDecimal');
            FuncSend(latInput);
            FuncSend(lonInput);
            closePopup();
        }
    </script>
</body>

</html>

)=====";