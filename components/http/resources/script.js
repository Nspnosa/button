async function getData(path) {
    const response = await fetch('/configuration' + path);
    return await response.json();
}

async function getHTMLComponent(component, data) {
    let innerHTML = "";
    let element = document.getElementById("content-container");

    switch (component) {
        case "security":
            innerHTML = `<input type="text" name="" id="security-ssid" value="${data.ssid}">\
                        <input type="text" name="" id="security-password" value="${data.password}">\
                        <h5 id="action-result"></h5>
                        <button id="security-button">Update</button>`
            element.innerHTML = innerHTML;
            document.getElementById("security-button").addEventListener("click", async function(event) {
                let actionResult = document.getElementById("action-result");
                let ssid = document.getElementById("security-ssid").value;
                let password = document.getElementById("security-password").value;
                let body = {"ssid": ssid, "password":password};
                // await fetch('/configuration/security', {method: "POST", body : JSON.stringify(body)});
                actionResult.innerText = JSON.stringify(body);
                console.log(body);
            });
            break;
        case "button":
            innerHTML = `<input type="number" name="" id="button-debounce" value="${data.debounceMs}">\
                        <input type="number" name="" id="button-actionDelay" value="${data.actionDelayMs}">\
                        <input type="number" name="" id="button-longPress" value="${data.longPressMs}">\
                        <h5 id="action-result"></h5>
                        <button id="configuration-button">Update</button>`
            element.innerHTML = innerHTML;
            document.getElementById("configuration-button").addEventListener("click", async function(event) {
                let actionResult = document.getElementById("action-result");
                let debounceMs = document.getElementById("button-debounce").value;
                let actionDelayMs = document.getElementById("button-actionDelay").value;
                let longPressMs = document.getElementById("button-longPress").value;
                let body = {"debounceMs": parseInt(debounceMs), "actionDelayMs": parseInt(actionDelayMs), "longPressMs": parseInt(longPressMs)};
                // await fetch('/configuration/security', {method: "POST", body : JSON.stringify(body)});
                actionResult.innerText = JSON.stringify(body);
                console.log(body);
            });
            break;
        case "provision":
            let options = "";
            for (let entry of data) {
                options = options + `<option value="${entry.ssid}">${entry.ssid}</option>`;
            }
            innerHTML = `<input type="text" name="product" list="ssid-list" id="provisioning-ssid" />
                        <input type="text" id="provisioning-password"/>
                        <datalist id="ssid-list">
                        ${options} 
                        </datalist>
                        <h5 id="action-result"></h5>
                        <button id="provision-button">Update</button>`
            element.innerHTML = innerHTML;
            document.getElementById("provision-button").addEventListener("click", async function(event) {
                let actionResult = document.getElementById("action-result");
                let ssid = document.getElementById("provisioning-ssid").value;
                let password = document.getElementById("provisioning-password").value;
                let body = {"ssid": ssid, "password": password};
                // await fetch('/configuration/credentials', {method: "POST", body : JSON.stringify(body)});
                actionResult.innerText = JSON.stringify(body);
                console.log(body);
            });
            break
        default:
            innerHTML = ``;
            element.innerHTML = innerHTML;
    }
} 

async function loadComponent(component, arg) {
    switch(component) {
        case "action": 
            const actionData = await getData(`/action/${arg}`);
            await getHTMLComponent(`action`, actionData);
            break;
        case "provision": 
            // const accessPointsList = await getData(`/ap`);
            //TODO: get configured ssid and password to display as default values
            const accessPointsList = [
                {
                    "ssid": "ssid1",
                    "security": 1,
                    "rssi": -23
                }, 
                {
                    "ssid": "ssid2",
                    "security": 1,
                    "rssi": -23
                }
            ]
            await getHTMLComponent("provision", accessPointsList);
            break;
        case "security": 
            // const securityConfiguration = await getData(`/servercredentials`);
            const securityConfiguration = {
                "ssid": "ssid",
                "password": "password",
            }
            await getHTMLComponent(`security`, securityConfiguration);
            break;
        case "button": 
            // const buttonConfiguration = await getData(`/button`);
            const buttonConfiguration = {
                "actionDelayMs": 300,
                "debounceMs": 4,
                "longPressMs": 400,
            }
            await getHTMLComponent(`button`, buttonConfiguration);
            break;
        default:
            await getHTMLComponent("", null);
            break;
    }
}

async function navigationEventListener(event) {
    const elements = document.getElementsByTagName("a");
    for (let element of elements) {
        element.className = "navigation";
    }
    event.target.className = "navigation-selected"
    await loadComponent(event.target.id);
}

window.onload = async (event) => {
    const elements = document.getElementsByClassName("navigation");
    for (let element of elements) {
        element.addEventListener("click", navigationEventListener);
    }
}