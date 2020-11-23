<script>
  import { onMount } from "svelte";
  import Chart from "chart.js";
  import fetch from "node-fetch"

  const chartData = {
    labels: ["Segunda", "Terça", "Quarta", "Quinta", "Sexta", "Sábado", "Domingo"],
    datasets: [
      {
        label: "ocupantes por dia da semana",
        data: [],
        backgroundColor: "rgba(3,117,220)"
      }
    ]
  };
  const chartOptions = {
    scales: {
      yAxes: [
        {
          ticks: {
            beginAtZero: true
          }
        }
      ]
    }
  };

  function createChart() {
    var ctx = document.getElementById("barChart");
    var myChart = new Chart(ctx, {
      type: "bar",
      data: chartData,
      options: chartOptions
    });
  }

  onMount(async () => {
    await fetch('http://192.168.15.49:3001/semanal')
      .then(r => r.json())
      .then(data => {
        chartData.datasets[0].data = data;
        createChart();
      });
  });

  async function refreshChart() {
    await fetch('http://192.168.15.49:3001/semanal')
      .then(r => r.json())
      .then(data => chartData.datasets[0].data = data)
  }

  const interval = setInterval(() => {
      refreshChart();
    }, 10000);
</script>

<canvas id="barChart" />
