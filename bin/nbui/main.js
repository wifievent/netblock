class Component {
  $target;
  $props;
  $state;
  constructor ($target, $props) { 
    return (async ($target, $props) => {
      this.$target = $target;
      this.$props = $props;
      await this.setup();
      this.setEvent();
      this.render();
    })($target, $props);
  }
  setup () {};
  mounted () {};
  template () { return ''; }
  render () {
    this.$target.innerHTML = this.template();
    this.mounted();
  }
  setEvent () {}
  setState (newState) {
    this.$state = { ...this.$state, ...newState };
    this.render();
  }
  addEvent (eventType, selector, callback) {
    const children = [ ...this.$target.querySelectorAll(selector) ]; 
    const isTarget = (target) => children.includes(target)
                                 || target.closest(selector);
    this.$target.addEventListener(eventType, event => {
      if (!isTarget(event.target)) return false;
      callback(event);
    })
  }
}

class Menu extends Component {
  template () {
    const {pages} = this.$props;

    let menu = ``;
    for (const page of pages) {
      const pageText = page.name.charAt(0).toUpperCase() + page.name.slice(1);
      menu += `
        <button 
          class='menu-button ${page.active ? 'red-button' : 'blue-button'}'
          value='${page.name}'>
          ${pageText}
        </button>`
    }
    return menu;
  }
  setEvent () {
    const { toggleMenu } = this.$props;
    this.addEvent('click', '.menu-button', ({target}) => {
      toggleMenu(target.value);
    });
  }
}

class DeviceList extends Component {
  template () {
    const {devices,selectedDeviceIndex} = this.$props;
    let deviceList = ``;
    for (const deviceId in devices) {
      deviceList += `
        <div class='device-list-row ${selectedDeviceIndex == deviceId ? 'active' : ''}' value='${devices[deviceId].host_id}'>
          <span class='device-list-id'>${devices[deviceId].host_id}</span>
          <span class='device-list-status text-center'>${devices[deviceId].isConnect ? 'O' : 'X'}</span>
          <span class='device-list-ip'>${devices[deviceId].ip}</span>
          <span class='device-list-name'>${devices[deviceId].nick_name || devices[deviceId].host_name || devices[deviceId].ip}</span>
        </div>
      `
    }
    return `
      <span class='title'>Device List</span>
      <div class='device-list'>
        <div class='device-list-row text-center'>
          <span class='device-list-id'>ID</span>
          <span class='device-list-status text-center'>On</span>
          <span class='device-list-ip'>IP</span>
          <span class='device-list-name'>Name</span>
        </div>
        ${deviceList}
      </div>
    `
  }

  setEvent () {
    const {selectDevice} = this.$props;

    this.addEvent('click', '.device-list-row:not(:first-child)', ({target}) => {
      selectDevice(parseInt(target.closest('.device-list-row').getAttribute('value')));
    })
  }
}

class DeviceInfoArea extends Component {
  template () {
    const {devices, selectedDeviceIndex} = this.$props;
    const device = devices[selectedDeviceIndex];
    return `
      <div class='device-info-row'>
        <span class='device-info-title'>OUI</span>
        <span class='device-info-out'>${device ? device.oui : ''}</span>
      </div>
      <div class='device-info-row'>
        <span class='device-info-title'>MAC</span>
        <span class='device-info-mac'>${device ? device.mac : ''}</span>
      </div>
      <div class='device-info-row'>
        <span class='device-info-title'>IP</span>
        <span class='device-info-ip'>${device ? device.ip : ''}</span>
      </div>
      <div class='device-info-row'>
        <span class='device-info-title'>Host Name</span>
        <span class='device-info-hostname'>${device ? device.host_name : ''}</span>
      </div>
      <div class='device-info-row'>
        <span class='device-info-title'>Nickname</span>
        <div class='device-info-nickname'>
          <input id='device-nick' type='text'/ value='${device ? device.nick_name : ''}'>
          <button class='blue-button'>Edit</button>
        </div>
      </div>
    `
  }

  setEvent () {
    const {selectedDeviceIndex,editNick} = this.$props;
    this.addEvent('click', '.device-info-nickname', ({target}) => {
      if (selectedDeviceIndex === -1) return;
      editNick(target.previousElementSibling.value);
    })
  }
}

class DeviceInfo extends Component {
  template () {
    return `
      <span class='title'>Device Information</span>
      <div class='device-info-area'></div>
      <div class='device-info-button-container'>
        <button id='device-delete-button' class='blue-button'>Delete</button>
      </div>
    `
  }

  mounted () {
    const {devices,selectedDeviceIndex,editNick} = this.$props;

    const $deviceInfoArea = this.$target.querySelector('.device-info-area');

    new DeviceInfoArea($deviceInfoArea, {
      devices,
      selectedDeviceIndex,
      editNick: editNick.bind(this),
    });
  }

  setEvent () {
    const {selectedDeviceIndex,deleteDevice} = this.$props;
    this.addEvent('click', '#device-delete-button', () => {
      if (selectedDeviceIndex === -1) return;
      deleteDevice();
    });
  }
}

class Device extends Component {
  template () {
    return `
      <div class='device-container'>
        <div class='device-list-container'></div>
        <div class='device-info-container'></div>
      </div>
    `
  }

  mounted () {
    const {devices,selectedDeviceIndex,selectDevice,editNick,deleteDevice} = this.$props;

    const $deviceList = this.$target.querySelector('.device-list-container');
    const $deviceInfo = this.$target.querySelector('.device-info-container');
    
    new DeviceList($deviceList, {
      devices,
      selectedDeviceIndex,
      selectDevice: selectDevice.bind(this),
    });
    new DeviceInfo($deviceInfo, {
      devices,
      selectedDeviceIndex,
      editNick: editNick.bind(this),
      deleteDevice: deleteDevice.bind(this),
    });
  }
}

class PolicyTable extends Component {
  template () {
    const {policies,selectedPolicyIndex} = this.$props;

    let policyTable = ``
    for (let hour = 0; hour < 24; hour++) {
      policyTable += `
        <div class='text-center'>${hour < 10 ? '0' : ''}${hour}</div>
      `
      for (let day = 0; day < 7; day++) {
        policyTable += `
          <div class='policy-table-item'></div>
        `
      }
    }

    const elem = document.createElement('div');
    elem.classList.add('policy-table-container')
    elem.innerHTML = policyTable;

    console.log('policies', policies);
    for (const policyId in policies) {
      const policy = policies[policyId];
      console.log('policy', policy);
      const startHour = parseInt(policy.start_time.slice(0, 2));
      const endMin = parseInt(policy.end_time.slice(2, 4));
      const endHour = parseInt(policy.end_time.slice(0, 2)) + (endMin > 30 ? 1 : 0);
      const day = parseInt(policy.day_of_the_week);
      for (let hour = startHour; hour < endHour; hour++) {
        const tableItem = elem.querySelector(
          `.policy-table-container > div:nth-child(${(day + 2) + (hour * 8)})`
        );
        tableItem.setAttribute('policy-id', policy.policy_id);
        if (parseInt(policyId) === selectedPolicyIndex) {
          tableItem.style.backgroundColor = 'var(--we-blue-color)';
          tableItem.style.borderColor = 'var(--we-red-color)';
        } else {
          tableItem.style.backgroundColor = 'grey';
          tableItem.style.borderColor = 'var(--body-background-color)';
        }
      }
    }
    return elem.innerHTML;
  }

  setEvent () {
    const {selectPolicy} = this.$props;
    this.addEvent('click', '.policy-table-item', ({target}) => {
      target.getAttribute('policy-id') && selectPolicy(parseInt(target.getAttribute('policy-id')));
    })
  }
}

class PolicyFilter extends Component {
  template () {
    const {devices,selectedDeviceIndex} = this.$props;

    let deviceFilter = ``;
    for (const deviceId in devices) {
      deviceFilter += `
        <option 
          id='policy-filter-item'
          value='${devices[deviceId].host_id}'
          ${selectedDeviceIndex == deviceId ? 'selected' : ''}>
          ${devices[deviceId].nick_name || devices[deviceId].ip}
        </option>
      `
    }

    return `
      <option disabled hidden selected>Select..</option>
      ${deviceFilter}
    `
  }

  setEvent () {
    const {selectDevice} = this.$props;
    this.addEvent('change', '#policy-filter', ({target}) => {
      selectDevice(parseInt(target.options[target.selectedIndex].value));
    })
  }
}

class PolicyButton extends Component {
  template () {
    return `
      <button id='policy-add' class='blue-button'>Add</button>
      <button id='policy-edit' class='blue-button'>Edit</button>
      <button id='policy-delete' class='red-button'>Delete</button>
    `
  }

  setEvent () {
    const {openPolicyConfig,deletePolicy} = this.$props;
    this.addEvent('click', 'button', ({target}) => {
      switch (target.getAttribute('id')) {
        case 'policy-add':
          openPolicyConfig(1);
          break;
        case 'policy-edit':
          openPolicyConfig(0);
          break;
        case 'policy-delete':
          deletePolicy();
      }
    })
  }
}

class PolicyConfig extends Component {
  template () {
    const {policies,selectedPolicyIndex,isPolicyConfig} = this.$props;

    const dayBoxType = isPolicyConfig === 1 ? 'radio' : 'checkbox';
    const dayOfTheWeek = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];
    let dayOfTheWeekView = ``;
    for (const dayId in dayOfTheWeek) {
      dayOfTheWeekView += `
        <div>
          <input type='${dayBoxType}' name='day' value='${dayId}'/>
          <label for='day'>${dayOfTheWeek[dayId]}</span>
        </div>
      `
    }

    let hourView = ``;
    for (let hour = 0; hour < 25; hour++) {
      hourView += `
        <option value='${hour}'>${hour}</option>
      `
    }

    let minView = ``;
    for (let min = 0; min < 51; min+=10) {
      minView += `
        <option value='${min}'>${min}</option>
      `
    }

    const policy = policies[selectedPolicyIndex];
    const elem = document.createElement('div');
    elem.setAttribute('id', 'policy-config-container');
    elem.innerHTML = `
    <div id='policy-config'>
      <h1>${isPolicyConfig === 1 ? policy.policy_id : 'new'}</h1>
      <div class='config-day-box'>
        ${dayOfTheWeekView}
      </div>
      <div class='config-time-container'>
        <div class='config-start-time'>
          <span>Start Time</span>
          <select class='config-hour'>${hourView}</select>
          <span>:</span>
          <select class='config-min'>${minView}</select>
        </div>
        <div class='config-end-time'>
          <span>End Time</span>
          <select class='config-hour'>${hourView}</select>
          <span>:</span>
          <select class='config-min'>${minView}</select>
        </div>
      </div>
      <div class='config-button-container'>
        <button id='config-apply' class='blue-button'>Apply</button>
        <button id='config-cancel' class='blue-button'>Cancel</button>
        <button id='config-delete' class='red-button'>Delete</button>
      </div>
    </div>
    `

    if (isPolicyConfig === 1) {
      const startHour = parseInt(policy.start_time.slice(0, 2));
      const startMin = parseInt(policy.start_time.slice(2, 4));
      const endHour = parseInt(policy.end_time.slice(0, 2));
      const endMin = parseInt(policy.end_time.slice(2, 4));
      const day = parseInt(policy.day_of_the_week);
      elem.querySelector('.config-start-time > .config-hour')[startHour].setAttribute('selected', 'selected');
      elem.querySelector('.config-start-time > .config-min')[startMin / 10].setAttribute('selected', 'selected');
      elem.querySelector('.config-end-time > .config-hour')[endHour].setAttribute('selected', 'selected');
      elem.querySelector('.config-end-time > .config-min')[endMin / 10].setAttribute('selected', 'selected');

      elem.querySelector(`.config-day-box > div > input[value='${day}']`).setAttribute('checked', 'checked');
    }

    return elem.innerHTML;
  }

  setEvent () {
    const {closePolicyConfig,createPolicy,updatePolicy,deletePolicy,isPolicyConfig} = this.$props;
    this.addEvent('click', '.config-button-container > button', ({target}) => {
      const startHour = this.$target.querySelector('.config-start-time > .config-hour > option:checked').value;
      const startMin = this.$target.querySelector('.config-start-time > .config-min > option:checked').value;
      const startTime = (startHour < 10 ? `0${startHour}` : startHour) + (startMin === '0' ? '00' : startMin);
      const endHour = this.$target.querySelector('.config-end-time > .config-hour > option:checked').value;
      const endMin = this.$target.querySelector('.config-end-time > .config-min > option:checked').value;
      const endTime = (endHour < 10 ? `0${endHour}` : endHour) + (endMin === '0' ? '00' : endMin);

      switch(target.getAttribute('id')) {
        case 'config-apply':
          if (isPolicyConfig === 1) {
            const day = parseInt(this.$target.querySelector(`.config-day-box > div > input:checked`).value);
            updatePolicy(startTime, endTime, day);
          } else {
            const dayList = this.$target.querySelectorAll(`.config-day-box > div > input:checked`);
            createPolicy(startTime, endTime, dayList);
          }
          break;
        case 'config-close':
          break;
        case 'config-delete':
          deletePolicy();
      }
      closePolicyConfig();
    })
  }
}

class Policy extends Component {
  async setup() {
    const {devices,selectedDeviceIndex} = this.$props;

    let policies = [];
    if (selectedDeviceIndex !== -1) {
      const data = {
        host_id: devices[selectedDeviceIndex].host_id
      }
      const getPolicyReq = await fetch('/getpolicy', {
        method: 'POST',
        body: JSON.stringify(data),
      });
      const policyRes = await getPolicyReq.json();
      policies = policyRes.policy || [];
    }

    this.$state = {
      policies,
      selectedPolicyIndex: -1,
      isPolicyConfig: 0,
    }
  }

  template () {
    return `
      <div id='policy-config-container'></div>
      <div class='policy-table-header'>
        <div class='text-center'>24</div>
        <div class='text-center'>Sun</div>
        <div class='text-center'>Mon</div>
        <div class='text-center'>Tue</div>
        <div class='text-center'>Wed</div>
        <div class='text-center'>Thu</div>
        <div class='text-center'>Fri</div>
        <div class='text-center'>Sat</div>
      </div>
      <div class='policy-table-container'></div>
      <div class='policy-menu-container'>
        <div class='policy-filter-container'>
          <span>Device Filter: </span>
          <select id='policy-filter'></select>
        </div>
        <div class='policy-button-container'></div>
      </div>
    `
  }

  mounted () {
    const {policies,selectedPolicyIndex,isPolicyConfig} = this.$state;
    const {selectPolicy,openPolicyConfig,closePolicyConfig,createPolicy,updatePolicy,deletePolicy} = this;
    const {devices,selectedDeviceIndex,selectDevice} = this.$props;

    const $policyConfig = this.$target.querySelector('#policy-config-container');
    const $policyTable = this.$target.querySelector('.policy-table-container');
    const $policyFilter = this.$target.querySelector('#policy-filter');
    const $policyButton = this.$target.querySelector('.policy-button-container');

    if (selectedDeviceIndex !== -1 && (isPolicyConfig === 1 && selectedPolicyIndex !== -1 || isPolicyConfig === 2)) {
      new PolicyConfig($policyConfig, {
        devices,
        selectedDeviceIndex,
        policies,
        selectedPolicyIndex,
        isPolicyConfig,
        closePolicyConfig: closePolicyConfig.bind(this),
        createPolicy: createPolicy.bind(this),
        updatePolicy: updatePolicy.bind(this),
        deletePolicy: deletePolicy.bind(this),
      });
    }
    new PolicyTable($policyTable, {
      devices,
      selectedDeviceIndex,
      policies,
      selectedPolicyIndex,
      selectPolicy: selectPolicy.bind(this),
    });
    new PolicyFilter($policyFilter, {
      devices,
      selectedDeviceIndex,
      selectDevice: selectDevice.bind(this),
    });
    new PolicyButton($policyButton, {
      openPolicyConfig: openPolicyConfig.bind(this),
      deletePolicy: deletePolicy.bind(this),
    });
  }

  selectPolicy (policy_id) {
    const {policies} = this.$state;
    this.setState({selectedPolicyIndex: policies.findIndex(p => p.policy_id === policy_id)});
  }

  openPolicyConfig (isNew) {
    this.setState({isPolicyConfig: 1 + isNew});
  }

  closePolicyConfig () {
    this.setState({isPolicyConfig: 0});
  }

  async createPolicy (start_time, end_time, dayList) {
    const {policies} = this.$state;
    const {devices,selectedDeviceIndex} = this.$props;
    const host_id = devices[selectedDeviceIndex].host_id;
    let policy = [];
    for (const day of dayList) {
      policy.push({
        start_time,
        end_time,
        day_of_the_week: parseInt(day.value),
      });
    }
    const data = {
      host_id,
      policy,
    }
    const createPolicyReq = await fetch('/policy', {
      method: 'POST',
      body: JSON.stringify(data)
    }).catch(err => {
      alert(err);
      return;
    });
    if (createPolicyReq.status !== 200) {
      alert('create policy failed');
      return;
    }
    alert('create policy success');
    policies.push(...policy);
    this.setState({ policies, selectedPolicyIndex: -1 });
  }
  
  async updatePolicy (start_time, end_time, day_of_the_week) {
    const {policies,selectedPolicyIndex} = this.$state;
    const policy_id = policies[selectedPolicyIndex].policy_id;
    const data = {
      policy_id,
      start_time,
      end_time,
      day_of_the_week
    }
    const updatePolicyReq = await fetch('/policy', {
      method: 'PATCH',
      body: JSON.stringify(data),
    }).catch(err => {
      alert(err);
      return;
    });
    if (updatePolicyReq.status !== 200) {
      alert('update policy failed');
      return;
    }
    alert('update policy success');
    policies[selectedPolicyIndex] = data;
    this.setState({ policies, selectedPolicyIndex: -1 });
  }

  async deletePolicy () {
    const {policies,selectedPolicyIndex} = this.$state;
    const policy_id = policies[selectedPolicyIndex].policy_id;
    const deletePolicyReq = await fetch('/policy', {
      method: 'DELETE',
      body: JSON.stringify({ policy_id }),
    }).catch(err => {
      alert(err);
      return;
    });
    if (deletePolicyReq.status !== 200) {
      alert('delete policy failed');
      return;
    }
    alert('delete policy success');
    policies.splice(selectedPolicyIndex, 1);
    this.setState({ policies, selectedPolicyIndex: -1 });
  }
}

class Footer extends Component {
  template () {
    return `
      <p>이 제품은 개인 사용자의 네트워크 제어 목적으로 만들어진 제품으로 공공장소 등에서 악의적인 목적으로 사용하거나 잘못된 사용으로 인한 손해가 발생한 경우 모든 법적 책임은 본인에게 있습니다</p>
      <p>You can give a free donation on the account number<br/>IBK 9730943757-01-011<br/>on name of the Lee Sungjin (Core Develospaner)</p>
    `
  }
}

class App extends Component {
  async setup () {
    const getDeviceReq = await fetch(`/device`, {
      method: 'GET'
    });
    const devices = await getDeviceReq.json();

    this.$state = {
      pages: [
        {
          name: 'device',
          active: true,
        },
        {
          name: 'policy',
          active: false,
        }
      ],
      devices,
      selectedDeviceIndex: -1,
    };
  }

  template () {
    return `
      <div class='menu-container'></div>
      <div class='content'></div>
      <div class='footer text-center'></div>
    `
  }

  mounted () {
    const { pages, devices, selectedDeviceIndex } = this.$state;
    const { toggleMenu, selectDevice, editNick, deleteDevice } = this;

    const $menu = this.$target.querySelector('.menu-container');
    const $device = this.$target.querySelector('.content');
    const $policy = this.$target.querySelector('.content');
    const $footer = this.$target.querySelector('.footer');

    new Menu($menu, {
      pages,
      toggleMenu: toggleMenu.bind(this),
    });
    switch (pages.find(p => p.active === true).name) {
      case 'device':
        new Device($device, {
          devices,
          selectedDeviceIndex,
          selectDevice: selectDevice.bind(this),
          editNick: editNick.bind(this),
          deleteDevice: deleteDevice.bind(this),
        });
        break;
      case 'policy':
        new Policy($policy, {
          devices,
          selectedDeviceIndex,
          selectDevice: selectDevice.bind(this),
        });
        break;
    }
    new Footer($footer);
  }

  toggleMenu (name) {
    const pages = [ ...this.$state.pages, ];
    const index = pages.findIndex(p => p.name === name);
    for (const page of pages) {
      page.active = false;
    }
    pages[index].active = true;
    this.setState({pages});
  }

  selectDevice (host_id) {
    const {devices} = this.$state;
    this.setState({selectedDeviceIndex: devices.findIndex(d => d.host_id === host_id)});
  }

  async editNick (nick) {
    const {devices,selectedDeviceIndex} = this.$state;
    const data = {
      host_id: devices[selectedDeviceIndex].host_id,
      nick_name: nick,
    }
    const patchDeviceNickReq = await fetch('/device', {
      method: 'PATCH',
      body: JSON.stringify(data),
    }).catch(err => {
      alert(err);
      return;
    });
    if (patchDeviceNickReq.status !== 200) {
      alert('edit nick failed');
      return;
    }
    alert('edit nick success');
    devices[selectedDeviceIndex].nick_name = nick;
    this.setState({devices});
  }

  async deleteDevice () {
    const {devices,selectedDeviceIndex} = this.$state;
    const data = {
      host_id: devices[selectedDeviceIndex].host_id,
    }
    const delDevReq = await fetch('/device', {
      method: 'DELETE',
      body: JSON.stringify(data),
    }).catch(err => {
      alert(err);
      return;
    });
    if (delDevReq.status !== 200) {
      alert('delete device failed');
      return;
    }
    alert('delete device success')
    devices.splice(selectedDeviceIndex, 1);
    this.setState({devices});
  }
}

new App(document.querySelector('#app'));