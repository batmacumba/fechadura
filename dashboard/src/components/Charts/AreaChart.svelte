<script>
  import { onMount } from "svelte";
  import Chart from "chart.js";

  const chartData = {
    labels: [ "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12",
              "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23"
    ],
    datasets: [
      {
        label: "ocupantes por hora do dia",
        data: [],
        borderWidth: 2,
        borderColor: "rgb(3,117,220)",
        backgroundColor: "rgba(3,117,220,0.2)"
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
    var ctx = document.getElementById("areaChart");
    var myChart = new Chart(ctx, {
      type: "line",
      data: chartData,
      options: chartOptions
    });
  }

  onMount(async () => {
    await fetch('http://localhost:3001/diaria')
      .then(r => r.json())
      .then(data => {
        chartData.datasets[0].data = data;
        createChart();
      });
  });

</script>

<canvas id="areaChart" />
