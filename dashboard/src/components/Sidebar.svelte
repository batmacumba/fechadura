<script>
  import Collapse from "sveltestrap/src/Collapse.svelte";
  import Nav from "sveltestrap/src/Nav.svelte";

  import SidebarItem from "./SidebarItem.svelte";

  export let segment;
  export let theme;

  $: sidenav_theme = `sb-sidenav-${theme}`;

  let isLayoutOpen = false;
  let isPageOpen = false;
  let isAuthenticationOpen = false;
  let isErrorOpen = false;
  let activeLink = "Dashboard";
  let footerName = "SB Admin Svelte";
  let footerText = "Logged in as:";

  const updateActiveLink = linkName => (activeLink = linkName);

  const toggleLayout = () => {
    isLayoutOpen = !isLayoutOpen;
    if (isPageOpen === true) isPageOpen = false;
  };

  const togglePages = () => {
    isPageOpen = !isPageOpen;
    if (isLayoutOpen === true) isLayoutOpen = false;
    if (isPageOpen === false) {
      isAuthenticationOpen = false;
      isErrorOpen = false;
    }
  };

  const toggleAuthentication = () => {
    isAuthenticationOpen = !isAuthenticationOpen;
    if (isErrorOpen === true) isErrorOpen = false;
  };

  const toggleError = () => {
    isErrorOpen = !isErrorOpen;
    if (isAuthenticationOpen === true) isAuthenticationOpen = false;
  };
</script>

<div id="layoutSidenav_nav" class="sb-nav-fixed">
  <Nav
    class="sb-sidenav {sidenav_theme} accordion sb-nav-fixed"
    id="sidenavAccordion">
    <div class="sb-sidenav-menu">
      <Nav>
        <div class="sb-sidenav-menu-heading">Core</div>
        <SidebarItem
          on:press={() => {
            theme = 'dark';
          }}
          text="Dashboard"
          class={segment === '.' || segment === undefined ? 'active' : ''}
          leftIcon
          href=".">
          <i class="fas fa-tachometer-alt" slot="leftIcon" />
        </SidebarItem>
      </Nav>
    </div>
  </Nav>
</div>
