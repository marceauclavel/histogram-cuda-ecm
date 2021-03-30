# histogram-cuda-ecm

Programme cuda permettant de calculer des histogrammes des occurences de lettres.
[lien de l'énoncé du projet](http://info.iut-bm.univ-fcomte.fr/staff/perrot/ECM-GPU/sujets/histoText/histoText.html)

## lancer le programme

Cloner le repo dans un dossier contenant le dossier common, et lancer la commande `make test` dans le dossier `src`. S'il n'y a aucune erreur, lancer le programme comme suit:
```bash
$ make
$ ./template -i <inputFile.txt> -o <outputFile.csv>
```

## Reflexions sur l'implémentation

Le programme lit dans un premier temps l'input pour compter le nombre de caractères. Après avoir allouer dynamiquement la mémoire nécessaire, les données sont stockées sur le host. Ensuite les données sont envoyées sur le device, et le comptage des occurences est parallélisé:
- Chaque thread s'occupe de `CHARS_PER_THREAD` caractères
- Chaque block fait tourner `THREADS_PER_BLOCK` threads

Au sein de chaque blocks, la mémoire partagés est utilisée.

> Avez-vous eu des difficultés à réaliser correctement l’optimisation ?

L'optimisation a été réalisée petit à petit, en partant du code en c. ensuite, voici les optimisations apportées:
- calcul sur le GPU
- répartition parmi blocks et threads
- utilisation de la mémoire partagée.

> Combien de lectures de la mémoire globale sont-elles effectuées par votre kernel calculant l’histogramme ? expliquez.

Le kernel parcours une fois toute la mémoire globale pour lire les caractères à compter. Un autre accès à la mémoire globale est fait 1 fois par block afin d'additionner le compteur de la mémoire partagée au compteur de la mémoire globale. Il y a donc `nChars + nCounts * nBlocks` pour `nChars` caractères au total, `nCounts` nombre de caractères ascii à compter et `nBlocks` blocks.

> Combien d’opérations atomiques sont effectuées par votre kernel calculant l’histogramme ? expliquez.

Au sein de chaque blocks, il y a une opération atomique par caractère contribuant à la mémoire partagée. Il y a ensuite un appel atomique lors de la copie de la mémoire partagée du block à la mémoire globale. Il y a donc `nChars + nCounts * nBlocks` pour `nChars` caractères au total, `nCounts` nombre de caractères ascii à compter et `nBlocks` blocks.


> La plupart des fichiers texte se composent uniquement de lettres, de chiffres et de caractères d’espacement. Que pouvons-nous dire sur les conflits d’accès concernant le nombre de threads qui essaient simultanément d’incrémenter atomiquement un histogramme privé ?

Les caractères ascii que l'on comptabilise est ici au nombre de 68, qui peut est très petit par rapport au nombre de caractères à compter. Il y aura donc un grnd taux de conflits d'accès. Ce nombre de conflits d'accès à la mémoire partagée croît linéairement avec le nombre de thread par block. Il faudra donc minimiser ce nombre de conflits en limitant le nombre de thread par block.
Il faudrait donc optimiser la répartission des caractères à analyser en trouvant le bon équilibre entre le nombre de  thread par block et le nombre de blocks.

Ce nombre de conflit d'accès pourrait être plus maîtriser en diminuant le nombre d'accès à la mémoire partagée, en utilisant la mémoire locale au sein de chaque thread avant de contribuer à la mémoire partagée.

## Pistes d'amélioration

Notre solution stocke tout l'input en mémoire dynamiquement. Celà permet d'allouer seulement la mémoire qu'il faut. Mais l'input peut être trop long, et ne pas rentrer dans la RAM. Une solution pourait être de traiter l'entrée par batchs, afin de maîtriser la taille d'allocation RAM maximale.
