#!/usr/bin/env python3

from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import Controller, OVSSwitch
from mininet.cli import CLI
from mininet.log import setLogLevel


class LeafSpineTopo(Topo):
   def __init__(self, k=4, **opts):
        assert k % 2 == 0, "k (switch radix) must be even"
        self.k = k
        super(LeafSpineTopo, self).__init__(**opts)
        self.build(k)

    def build(self, k=4):
        num_spines = k // 2
        num_leaves = k
        hosts_per_leaf = k // 2

        spines = []
        leaves = []

        
        for i in range(1, num_spines + 1):
            sw = self.addSwitch(f"s{i}")
            spines.append(sw)

        
        for i in range(1, num_leaves + 1):
            sw = self.addSwitch(f"l{i}")
            leaves.append(sw)

        
        for leaf in leaves:
            for spine in spines:
                self.addLink(leaf, spine)

        
        for leaf_idx, leaf in enumerate(leaves, start=1):
            for h in range(1, hosts_per_leaf + 1):
                host = self.addHost(f"h{leaf_idx}_{h}")
                self.addLink(host, leaf)


def run_demo(k=4):
   topo = LeafSpineTopo(k=k)
    net = Mininet(
        topo=topo,
        controller=Controller,
        switch=OVSSwitch,
        autoSetMacs=True,
        autoStaticArp=True
    )

    net.start()
    print("\n*** Network started with k =", k)
    print("*** Hosts:", " ".join(sorted(net.hosts.keys())))

    print("\n*** Ping test between all hosts...")
    net.pingAll()

    print("\n*** Dropping to CLI (type 'exit' to stop Mininet)")
    CLI(net)

    net.stop()

topos = {
    "leafspine": lambda k=4: LeafSpineTopo(k=int(k))
}

if __name__ == "__main__":
    setLogLevel("info")
    run_demo(k=4)
