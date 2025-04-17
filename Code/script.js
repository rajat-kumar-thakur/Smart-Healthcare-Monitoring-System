const hrCtx = document.getElementById('hrChart').getContext('2d');
const spo2Ctx = document.getElementById('spo2Chart').getContext('2d');

const hrChart = new Chart(hrCtx, {
  type: 'line',
  data: {
    labels: [],
    datasets: [{
      label: 'HR',
      data: [],
      fill: true,
      tension: 0.3,
      borderColor: 'var(--primary)',
      backgroundColor: 'rgba(0,121,108,0.1)'
    }]
  },
  options: {
    responsive: true,
    maintainAspectRatio: false,
    scales: { y: { beginAtZero: true } }
  }
});

const spo2Chart = new Chart(spo2Ctx, {
  type: 'line',
  data: {
    labels: [],
    datasets: [{
      label: 'SpO2',
      data: [],
      fill: true,
      tension: 0.3,
      borderColor: '#1e88e5',
      backgroundColor: 'rgba(30,136,229,0.1)'
    }]
  },
  options: {
    responsive: true,
    maintainAspectRatio: false,
    scales: { y: { beginAtZero: true } }
  }
});

async function fetchData() {
  const d = await fetch('/data').then(r => r.json());
  document.getElementById('roomTemp').textContent = d.roomTemp.toFixed(1) + ' °C';
  document.getElementById('roomHum').textContent = d.roomHumidity.toFixed(1) + ' %';
  document.getElementById('bodyTemp').textContent = ((d.dsTemp * 9 / 5) + 32).toFixed(1) + ' °F';

  const t = new Date().toLocaleTimeString();
  hrChart.data.labels.push(t);
  hrChart.data.datasets[0].data.push(d.heartRate);
  spo2Chart.data.labels.push(t);
  spo2Chart.data.datasets[0].data.push(d.spo2);

  if (hrChart.data.labels.length > 30) {
    hrChart.data.labels.shift();
    hrChart.data.datasets[0].data.shift();
  }
  if (spo2Chart.data.labels.length > 30) {
    spo2Chart.data.labels.shift();
    spo2Chart.data.datasets[0].data.shift();
  }

  hrChart.update();
  spo2Chart.update();
}

setInterval(fetchData, 1000);
