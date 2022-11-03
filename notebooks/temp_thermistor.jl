### A Pluto.jl notebook ###
# v0.19.14

using Markdown
using InteractiveUtils

# ╔═╡ 471fc10b-f9a2-45c0-8c18-c731873b0f73
begin
	import Pkg
	Pkg.activate(".")
end

# ╔═╡ 3c282cfe-614a-4f5c-b51a-49916756f282
begin
	using CairoMakie
	using Hotwire
end

# ╔═╡ 65317174-523b-11ed-0579-b91b8d0a826a
md"""

# Temperature measurement with Thermistors
"""

# ╔═╡ 0cc3acf7-4eec-4bb7-8c7f-cbf26a458d6a
md"""
## Simplest circuit

The thermistor is in series with the resistor. It can be located before (top) or after (bottom) the fixed resistor.
"""

# ╔═╡ 03fce474-1057-434f-95e6-d97d99419cca
begin
	Rₜ = Thermistor(10_000, 3500, 298.15)
	R₁ = 5_000.0
	Eᵢ = 2.5 # Input voltage
end

# ╔═╡ 245fc30d-0861-4a8a-baa7-d53eb31a0b33
begin
	Tc = -30.0:100.0
	Tk = Tc .+ 273.15
end

# ╔═╡ df693e74-60a7-4a80-b072-57adf16b5ca4
R = Rₜ.(Tk)

# ╔═╡ cf1961c6-a78c-4810-8aad-812c49e69811
i = Eᵢ ./ (R₁ .+ R)

# ╔═╡ f07c2fa0-4e08-4041-96fd-2d0c7d63d519
Eₒ = R₁ .* i

# ╔═╡ abae724d-e8a4-4ff5-a594-31f7a2378fa7
let
	fig = Figure()
	ax1 = Axis(fig[1,1], xlabel="Temperature (°C)", ylabel="Output (V)", title="Thermistor on top")
	lines!(ax1, Tc, i .* R₁)

	ax2 = Axis(fig[1,2], xlabel="Temperature (°C)", ylabel="Output (V)", title="Thermistor on bottom")
	lines!(ax2, Tc, i .* R)

	fig
end


# ╔═╡ 671984dc-b81c-4c00-b638-2675b82bbfb1
md"""
## Let's use w wheatstone bridge!

"""

# ╔═╡ bf96e2f5-952f-4c52-8c76-9952f8967af9
begin
	Ra = 10_000.0
	Rb = 10_000.0
	Rc = 1_000.0

	Eref = Eᵢ * Rc / (Rb + Rc)
end


# ╔═╡ 4d16682b-e9cf-42b5-9061-7cf7076a34e9
Eth = Eᵢ .* R ./ (R .+ Ra)

# ╔═╡ bd33fe12-c2da-4890-a6b5-5cb0f09845d7
lines(Tc, Eth .- Eref)

# ╔═╡ Cell order:
# ╠═65317174-523b-11ed-0579-b91b8d0a826a
# ╠═471fc10b-f9a2-45c0-8c18-c731873b0f73
# ╠═3c282cfe-614a-4f5c-b51a-49916756f282
# ╠═0cc3acf7-4eec-4bb7-8c7f-cbf26a458d6a
# ╠═03fce474-1057-434f-95e6-d97d99419cca
# ╠═245fc30d-0861-4a8a-baa7-d53eb31a0b33
# ╠═df693e74-60a7-4a80-b072-57adf16b5ca4
# ╠═cf1961c6-a78c-4810-8aad-812c49e69811
# ╠═f07c2fa0-4e08-4041-96fd-2d0c7d63d519
# ╠═abae724d-e8a4-4ff5-a594-31f7a2378fa7
# ╠═671984dc-b81c-4c00-b638-2675b82bbfb1
# ╠═bf96e2f5-952f-4c52-8c76-9952f8967af9
# ╠═4d16682b-e9cf-42b5-9061-7cf7076a34e9
# ╠═bd33fe12-c2da-4890-a6b5-5cb0f09845d7
