<script>
  import { onMount } from "svelte";
  import Chart from "chart.js";

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
    await fetch('http://localhost:3001/semanal')
      .then(r => r.json())
      .then(data => {
        chartData.datasets[0].data = data;
        createChart();
      });
  });
</script>

<canvas id="barChart" />
