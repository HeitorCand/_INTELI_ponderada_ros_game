{
  description = "Dev shell ROS 2 – Culling Games";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.05";
    ros-overlay.url = "github:lopsided98/nix-ros-overlay";
  };

  outputs = { self, nixpkgs, ros-overlay }:
    let
      linuxSystem = "x86_64-linux";
      linuxPkgs = import nixpkgs {
        system = linuxSystem;
        overlays = [ ros-overlay.overlays.default ];
      };
      darwinSystem = "aarch64-darwin";
      darwinPkgs = import nixpkgs { system = darwinSystem; };
    in {
      devShells.${linuxSystem}.default = linuxPkgs.mkShell {
        buildInputs = with linuxPkgs; [
          rosPackages.humble.desktop
          rosPackages.humble.colcon-common-extensions
          python3Packages.pygame
          python3Packages.numpy
        ];
        shellHook = ''
          source ${linuxPkgs.rosPackages.humble.desktop}/setup.bash
          export ROS_LOCALHOST_ONLY=1
        '';
      };

      devShells.${darwinSystem}.default = darwinPkgs.mkShell {
        shellHook = ''
          echo "[cg] ROS 2 via Nix só está disponível em x86_64-linux."
          echo "[cg] Use \"nix develop --system ${linuxSystem}\" dentro de um container/VM Linux (docker, colima, WSL, etc.)."
          echo "[cg] Este shell Darwin contém apenas utilitários básicos do nixpkgs."
        '';
      };
    };
}
