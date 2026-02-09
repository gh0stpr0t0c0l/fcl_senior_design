params = scf.cf.param.toc.toc
print("Number of param groups:", len(params))

for group in list(params.keys())[:5]:
    print(group, params[group].keys())
