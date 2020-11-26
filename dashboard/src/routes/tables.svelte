<script>
  import Card from "sveltestrap/src/Card.svelte";
  import CardBody from "sveltestrap/src/CardBody.svelte";
  import CardHeader from "sveltestrap/src/CardHeader.svelte";
  import CardFooter from "sveltestrap/src/CardFooter.svelte";
  import Form from "sveltestrap/src/Form.svelte";
  import FormGroup from "sveltestrap/src/FormGroup.svelte";
  import Label from "sveltestrap/src/Label.svelte";
  import Input from "sveltestrap/src/Input.svelte";
  import CustomInput from "sveltestrap/src/CustomInput.svelte";
  import Button from "sveltestrap/src/Button.svelte";
  import fetch from "node-fetch"

  let uid_desconhecido = "";
  // onMount(async () => {
  //   await fetch('http://192.168.15.49:3001/desconhecido')
  //     .then(r => r.json())
  //     .then(data => {
  //       uid_desconhecido = data;
  //     });
  // });

  async function refreshUID() {
    await fetch('http://192.168.15.49:3001/desconhecido')
      .then(r => r.json())
      .then(data => {
        uid_desconhecido = data;
      });
  }

  async function autoriza() {
    await fetch('http://192.168.15.49:3001/autoriza');
    uid_desconhecido = "";
  }

  async function nao_autoriza() {
    await fetch('http://192.168.15.49:3001/nao_autoriza');
    uid_desconhecido = "";
  }

  const interval = setInterval(() => {
      refreshUID();
    }, 500);
</script>
<br><br>

<center>
{#if uid_desconhecido != ""}
<div class="col-lg-5">
  <Card class="shadow-lg border-0 rounded-lg mt-5">
    <CardHeader>
      <h3 class="text-center font-weight-light my-4">Autoriza o UID abaixo?</h3>
    </CardHeader>
    <CardBody>
      <Form>
        <FormGroup>
          <Input
            class="py-4"
            type="email"
            name="email"
            id="exampleEmail"
            placeholder="{uid_desconhecido}" />
        </FormGroup>
        <FormGroup
          class="d-flex align-items-center justify-content-between mt-4 mb-0">
          <Button color="success" href="." on:click={autoriza}>Sim</Button>
          <Button color="danger" href="." on:click={nao_autoriza}>Não</Button>
        </FormGroup>
      </Form>
    </CardBody>
  </Card>
</div>
{:else}
<div class="col-lg-5">
  <Card class="shadow-lg border-0 rounded-lg mt-5">
    <CardBody>
      <h3 class="text-center font-weight-light my-4">Aguardando leitura do cartão</h3>
    </CardBody>
  </Card>
</div>
{/if}
</center>

