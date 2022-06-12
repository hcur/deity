{ pkgs ? import <nixpkgs> {} }:

# a .nix file I used to test deity on my nixos machine.
# likely worthless to any end user/non-nixos-resident.

pkgs.mkShell {
  name = "deity";
  buildInputs = with pkgs; [
    xorg.libX11
  ];

  shellHook = ''
            echo "Starting new shell...";
  '';
}
