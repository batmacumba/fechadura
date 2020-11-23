<script>
  import Breadcrumb from "sveltestrap/src/Breadcrumb.svelte";
  import BreadcrumbItem from "sveltestrap/src/BreadcrumbItem.svelte";
  import Card from "sveltestrap/src/Card.svelte";
  import CardBody from "sveltestrap/src/CardBody.svelte";
  import CardHeader from "sveltestrap/src/CardHeader.svelte";
  import CardText from "sveltestrap/src/CardText.svelte";
  import CardSubtitle from "sveltestrap/src/CardSubtitle.svelte";
  import Row from "sveltestrap/src/Row.svelte";

  import DashboardCard from "../components/DashboardCard.svelte";
  import CustomCard from "../components/CustomCard.svelte";
  import Table from "../components/Table.svelte";
  import AreaChart from "../components/Charts/AreaChart.svelte";
  import BarChart from "../components/Charts/BarChart.svelte";

  let title = "Dashboard - Fechadura Inteligente";
</script>

<svelte:head>
  <title>{title}</title>
</svelte:head>
<CustomCard cardTitle="Ocupação Instantânea" cardIcon="fas fa-table">
  <Table />
</CustomCard>
<Row>
  <div class="col-xl-6">
    <CustomCard cardTitle="Ocupação por hora do dia" cardIcon="fas fa-chart-area">
      <AreaChart />
    </CustomCard>
  </div>
  <div class="col-xl-6">
    <CustomCard cardTitle="Ocupação por dia da semana" cardIcon="fas fa-chart-bar">
      <BarChart />
    </CustomCard>
  </div>
</Row>

