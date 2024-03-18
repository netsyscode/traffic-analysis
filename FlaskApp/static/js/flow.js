function fetchData() {
    // This URL is just an example, you should replace it with your actual backend endpoint
    fetch('http://your-backend-endpoint/api/data')
    .then(response => response.json())
    .then(data => {
        populateData(data);
    })
    .catch(error => console.error('Error fetching data:', error));
}

function populateData(data) {
    const container = document.querySelector('.data-container');

    // Assuming data is an array of objects like:
    // [{ name: "App", users: 8390, percentage: 1.98, new: 22, total: 368 }, ...]

    data.forEach(item => {
        const dataRow = document.createElement('div');
        dataRow.className = 'data-row';
        
        dataRow.innerHTML = `
            <div class="data-title">${item.name}</div>
            <div class="data-percentage">${item.percentage}%</div>
            <div class="data-numbers">
                <span>${item.new}</span>
                <span>${item.total}</span>
            </div>
        `;
        container.appendChild(dataRow);
    });
}
