<script>
  import Table from "sveltestrap/src/Table.svelte";
  import { onMount } from "svelte";
  let instantanea = 0;
  onMount(async () => {
    await fetch('http://localhost:3001/instantanea')
      .then(r => r.json())
      .then(data => {
        instantanea = data;
      });
  })
</script>

<style>
  p {
    text-align: center;
  }
  img {
    width: 5%; /* Will shrink image to 30% of its original width */
    height: auto;    
  }​{}
</style>

<Table bordered responsive>
  <tbody>
    <p>
    {#each { length: instantanea } as _, i}
    <img src='ocupante.png' />
    {/each}
    </p>
    <p> {instantanea} pessoas </p>
  </tbody>
</Table>
