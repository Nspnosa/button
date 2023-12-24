async function getData(path) {
    const response = await fetch('/configuration' + path);
    return await response.json();
}

async function getHTMLComponent(data) {
    fetch(`${page}.html`)
        .then(response => response.text())
        .then(html => {
            document.getElementById('content-container').innerHTML = html;
        })
        .catch(error => console.error('Error fetching page:', error));
} 

function hidrateActionComponent(actionData) {
}

async function loadComponent(component, arg) {
    switch(component) {
        case "action": 
            const actionData = await getData(`/action/${arg}`);
            await getHTMLComponent(`action`);
            hidrateActionComponent(actionData);
            break;
        default:
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