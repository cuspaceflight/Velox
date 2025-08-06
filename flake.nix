{
  description = "Flake for Velox groundstation";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.05";

  outputs = { self, nixpkgs, ... }:
    let
      system = "x86_64-linux";

      nixpkgs-esp-dev = builtins.fetchGit {
        url = "https://github.com/mirrexagon/nixpkgs-esp-dev.git";
        rev = "6c34f2436015eb6c107970d9b88f3d5d4600c6fa";
      };

      pkgs = import nixpkgs {
        inherit system;
        overlays = [ (import "${nixpkgs-esp-dev}/overlay.nix") ];
      };
    in {
      devShells.${system}.default = pkgs.mkShell {
        buildInputs = with pkgs; [
          esp-idf-full

          minicom
          cmake
        ];

        shellHook = ''
          export GIT_CONFIG_GLOBAL=$(realpath .gitconfig)
          git config --global --add safe.directory '${pkgs.esp-idf-full}'
          echo "git config --global --add safe.directory '${pkgs.esp-idf-full}'"
        '';
      };
    };
}
