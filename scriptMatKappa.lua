local grp1, grp2 = {} , {}

for line in io.lines("groupe1/corpus.all") do
	local id, lbl = line:match("(%d+),([^,]+)")
	grp1[id] = lbl
end

for line in io.lines("groupe2/corpus.all") do
	local id, lbl = line:match("(%d+),([^,]+)")
	grp2[id] = lbl
end

-- matrice de contingence
local mat = {
	pos = {pos=0, neu=0, neg=0, irr=0},
	neu = {pos=0, neu=0, neg=0, irr=0},
	neg = {pos=0, neu=0, neg=0, irr=0},
	irr = {pos=0, neu=0, neg=0, irr=0}
}
local lbls = {"pos", "neu","neg", "irr"}

local cnt = 0 -- tweet en commun
for id, lbl1 in pairs(grp1) do
	if grp2[id] then
		local lbl2 = grp2[id]
		mat[lbl1][lbl2] = mat[lbl1][lbl2]+1
		cnt = cnt+1
	end
end

local ok = mat["pos"]["pos"] + mat["neu"]["neu"] + mat["neg"]["neg"] + mat["irr"]["irr"]

local p0 = ok / cnt
for _,e in ipairs(lbls) do 
	local n_pi, n_ip = 0, 0
	for_, i in ipairs(lbls) do 
		n_pi = n_pi + mat[i][e]
		n_ip = n_ip + mat[e][i]
	end
	pe = pe + n_pi * n_ip
end
pe = pe / (cnt * cnt)
local kappa (p0 - pe) / (1-pe)

for lbl1 in ipairs(lbls) do
	io.write("\t")
	for lbl2 in pairs(lbls) do
		io.write(mat[lbl1][lbl2]) 
	end
end


